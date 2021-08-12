#include "Thingify.h"
#include <functional>
#include "Helpers/StringHelper.h"
#include "Modules/DiagnosticsModule.h"
#include "Modules/StatusLedModule.h"
#include "ThingifyConstants.h"
#include "Api/HeartbeatPacket.h"
#include "Api/DeviceNodeUpdateResult.h"
#include "Api/ClientReceivedSessionCreateAckPacket.h"

#include <EEPROM.h>
using namespace std::placeholders;

Thingify::Thingify(const char *deviceName, IAsyncClient& client) :
_mqtt(client),
_currentState(ThingState::Disabled),
_incomingPackets(0),
_valueSendInterval(0),
_errorType(ThingError::NoError),
_packetSender(_mqtt),
_logger(LoggerInstance),
_deviceName(deviceName), 
_firmwareUpdateService(_packetSender, _settingsStorage)
,NodeCollection(&_settings)
{
	_modules.reserve(16);
	_publishedNodeCount = 0;	

	SubscribeToEvents();	
	_outgoingPacketId = 0;
	_stateChangeTimer.Start();
	WatchdogEnabled = true;

	_logger.info(L("Sizeof NodeValue = %d"), sizeof(NodeValue));
	_logger.info(L("Sizeof UpdateNodesFromClientPacket: %d"), sizeof(UpdateNodesFromClientPacket));
	_logger.info(L("Size of function execution buffer: %d"), sizeof(FixedList<FunctionExecutionResponseItem, ThingifyConstants::MaxFunctionExecutionRequests>));
}
void Thingify::SetToken(const char* token)
{
	_settings.Token = token;
	_settings.ApiPort = 1883;
	_settings.ApiServer = "conti.ml";
	_isUsingManualConfiguration = true;
}

void Thingify::Start()
{
	_logger.info(L("Thing::Start"));
	_errorType = ThingError::NoError;
	EEPROM.begin(100);
	ThingifyUtils::ReadRestartReason(_restartReason);
	_loopWatchdog.Start(WatchdogTimeoutInMs());
	if (_restartReason.length() > 0)
	{
		_logger.err(L("Restart reason: %s"), _restartReason.c_str());
	}
	ThingifyUtils::ClearRestartReason();

	_logger.info(L("Module count: %d"), _modules.size());
	for (auto module : _modules)
	{
		_logger.info(L("Initializing module %s"), module->GetName());
		if (!module->Init())
		{
			_logger.err(L("Failed to initialize module '%s': %s"), module->GetName(), module->GetError());
		}
	}
	LogNodes();
	StartInternal();	
}

void Thingify::StartInternal()
{
	
	if(!_settingsStorage.Get(_settings))
	{
		if(_isUsingManualConfiguration)
		{
			_logger.info(F("Using manual configuration"));
		}
		else
		{
			_logger.info(F("Failed to read configuration"));
			SetState(ThingState::Configuring);
			StartZeroConfiguration();
			return;	
		}
	}
	else
	{
		_logger.info(F("Configuration read successfull"));
	}
	
	
   
	ThingifyUtils::LogSettings(_logger, _settings);
	OnConfigurationLoaded();
	if (IsNetworkConnected())
	{
		_logger.debug(L("Call ConnectToServer from Thingify::Start"));
		ConnectToServer();
	}
	else
	{
		_logger.info(LogComponent::Network, L("wlan not connected, waiting for network"));
		SetState(ThingState::SearchingForNetwork);
	}
}

void Thingify::Stop()
{
	_logger.info(L("Thing::Stop"));
	SetState(ThingState::Disabled);
	DisconnectMqtt();
}

void Thingify::ResetConfiguration()
{
	_logger.info(L("Thingify::ResetConfiguration"));
	Stop();
	_settingsStorage.Clear();
	SetState(ThingState::Configuring);
	StartZeroConfiguration();
}

void Thingify::OnConfigurationLoaded()
{

}
void Thingify::Loop()
{
	_loopWatchdog.Feed();
	for (auto module : _modules)
	{
		module->Tick();
	}

	CheckErrors();
	HandleWatchdog();

	if(_currentState == ThingState::Configuring)
	{
		if(IsZeroConfigurationReady())
		{
			StartInternal();
		}
	}

	if (_currentState == ThingState::DisconnectedFromMqtt)
	{
		if ((millis() - _disconnectedTimer) > 3000)
		{
			_logger.warn(L("Disconnected from mqtt timer elapsed"));
			ConnectToServer();
		}
	}

	if (_currentState == ThingState::ConnectingToMqtt)
	{
		if (_stateChangeTimer.ElapsedMs() > ThingifyConstants::MqttConnectingTimeout)
		{
			_logger.warn(L("Connecting to mqtt timer elapsed"));
			_disconnectedTimer = millis();
			SetState(ThingState::DisconnectedFromMqtt);
		}
	}
	if (_currentState == ThingState::Authenticating)
	{
		if (_stateChangeTimer.ElapsedMs() > ThingifyConstants::AuthenticatingTimeout)
		{
			_stateChangeTimer.Start();
			_logger.warn(L("Authenticating timeout"));
			Authenticate();
		}
	}

	if (!_mqtt.connected())
	{
		return;
	}

	if (_currentState == ThingState::Online)
	{
		static SoftTimer valueSendTimer(_valueSendInterval);
		valueSendTimer.UpdateDelay(_valueSendInterval);

		bool shouldSendValueUpdates = valueSendTimer.IsElapsed();

		if (shouldSendValueUpdates)
		{
			SendNodeValues();
		}

		_firmwareUpdateService.Loop();
	}
}

void Thingify::SubscribeToEvents()
{
	_mqtt.onConnect(bind(&Thingify::onMqttConnected, this, _1));
	_mqtt.onDisconnect(bind(&Thingify::onMqttDisconnect, this, _1));
	_mqtt.onMessage(bind(&Thingify::onMqttMessage, this, _1, _2, _3, _4, _5, _6));
	_mqtt.onSubscribe(bind(&Thingify::onMqttSubscribe, this, _1));
	_mqtt.onPublish(bind(&Thingify::onMqttPublishAcknowlendged, this, _1));	
}
void Thingify::SetError(const char* errorStr)
{
	SetError(ThingError::Other, errorStr);
}
void Thingify::SetError(const __FlashStringHelper* errorStr)
{
	FixedString64 errorTmp;	
	errorTmp = errorStr;
	SetError(ThingError::Other, errorTmp.c_str());
}

void Thingify::SetError(ThingError error, const char* errorStr)
{
	SetState(ThingState::Error);

	if (_currentState == ThingState::Online)
	{		
		DisconnectMqtt();
	}
	if (error != ThingError::Other)
	{
		_errorStr = ThingifyUtils::ThingErrorToStr(error);
	}
	if (errorStr != nullptr)
	{
		_errorStr = errorStr;
	}
	_logger.err(F("SetError: %s"), _errorStr.c_str());

	_errorType = error;
}

void Thingify::OnNetworkConnecting()
{
	_logger.info(LogComponent::Network, L("Thingify::OnNetworkConnecting: '%s'"), GetNetworkName().c_str());
	if (HasTerminalState(F("OnNetworkConnecting")))
	{
		return;
	}	
	SetState(ThingState::ConnectingToNetwork);
}

void Thingify::OnNetworkConnected()
{
	_logger.info(LogComponent::Network, L("Thingify::OnNetworkConnected"));
	if (HasTerminalState(F("OnNetworkConnected")))
	{
		return;
	}
	if (_currentState == ThingState::ConnectingToNetwork ||
		_currentState == ThingState::DisconnectedFromMqtt ||
		_currentState == ThingState::ConnectingToMqtt ||
		_currentState == ThingState::SearchingForNetwork)
	{
		_logger.debug(L("Call ConnectToServer from Thingify::OnNetworkConnected"));

		ConnectToServer();
	}
}

void Thingify::OnNetworkDisconnected()
{
	_logger.info(LogComponent::Network, L("Thingify::OnNetworkDisconnected"));
	if (HasTerminalState(F("OnNetworkDisconnected")))
	{
		return;
	}
	SetState(ThingState::SearchingForNetwork);
	DisconnectMqtt();	
}

void Thingify::Authenticate()
{
	auto requestId = StringHelper::GenerateRandomString<10>();
	_logger.debug(L("Generated request id: %s"), requestId.c_str());

	auto outTopic = FixedString64(ThingifyConstants::LoginRequestTopicPrefix) + requestId;
	_packetSender.SetOutTopic(outTopic);
	_inTopic = FixedString64(ThingifyConstants::LoginResponseTopicPrefix) + requestId;
	
	const uint16_t packetIdSub = _mqtt.subscribe(_inTopic.c_str(), 2);
	_logger.debug(L("Subscribed to %s, subsId = %d"), _inTopic.c_str(), packetIdSub);

	auto thingSessionCreatePacket = new ThingSessionCreatePacket;
	thingSessionCreatePacket->LoginToken = requestId;
	thingSessionCreatePacket->ClientId = _settings.Token;
	thingSessionCreatePacket->DeviceName = _deviceName;
	thingSessionCreatePacket->FirmwareVersion = "1.0.0";
	thingSessionCreatePacket->Nodes = GetWorkingNodes();

	SendAndDeletePacket(thingSessionCreatePacket);
}

bool Thingify::SendAndDeletePacket(PacketBase* packet)
{
	auto result = _packetSender.SendPacket(packet);
	delete packet;
	if (!result)
	{
		DisconnectMqtt();
	}
	return result;
}

void Thingify::onMqttConnected(bool sessionPresent)
{
	if (HasTerminalState(F("onMqttConnected")))
	{
		return;
	}

	_logger.info(LogComponent::Network, L("onMqttConnected, sessionPresent = %d"), sessionPresent);
	_reconnectCount++;
	SetState(ThingState::ServiceUnavailable);

	const uint16_t serverStatusSubsId = _mqtt.subscribe(ThingifyConstants::ServerStatusTopic, 2);
	_logger.debug(L("Subscribed to %s, subsId = %d"), ThingifyConstants::ServerStatusTopic, serverStatusSubsId);
}

void Thingify::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
	_logger.info(LogComponent::Network, L("onMqttDisconnect, reason: %d"), reason);	
	
	if (HasTerminalState(F("onMqttDisconnect")))
	{
		return;
	}

	if (_currentState != ThingState::SearchingForNetwork &&
		_currentState != ThingState::ConnectingToNetwork)
	{
		_disconnectedTimer = millis();
		SetState(ThingState::DisconnectedFromMqtt);
	}
}

void Thingify::ConnectToServer()
{
	_lastWillTopic.clear();
	_lastWillTopic.appendFormat("%s%s", ThingifyConstants::LastWillTopicPrefix, _settings.Token.c_str());

	_mqtt.
		setKeepAlive(ThingifyConstants::MqttKeepAliveInSeconds).
		setCredentials( _settings.Token.c_str(), "").
		setClientId( _settings.Token.c_str());

	_mqtt.setWill(_lastWillTopic.c_str(), 2, false, "lw", 2);
	_mqtt.setServer(_settings.ApiServer.c_str(), _settings.ApiPort);

	SetState(ThingState::ConnectingToMqtt);
	_logger.info(LogComponent::Network, L("Connecting to MQTT server: %s:%d"), _settings.ApiServer.c_str(), _settings.ApiPort);
	_logger.info(L("Last will: %s"), _lastWillTopic.c_str());
	_mqtt.connect();	
}

void Thingify::DisconnectMqtt()
{
	_logger.info(L("DisconnectMqtt"));
	_logger.info(L("Sending last will to %s"), _lastWillTopic.c_str());
	_mqtt.publish(_lastWillTopic.c_str(), 2, false, "lw", 2);
	delay(200);
	if (!_mqtt.connected())
	{
		_logger.info(L("DisconnectMqtt: mqtt already disconnected"));
		return;
	}
	_logger.info(L("DisconnectMqtt: call mqtt.disconnect()"));
	_mqtt.disconnect();
}



void Thingify::onMqttSubscribe(uint16_t packetId)
{
	_logger.debug(LogComponent::Mqtt, L("Subscribe ACK received: packetId: %d"), packetId);
}

void Thingify::HandlePacket(PacketBase *packet)
{
	_lastPacketReceivedTimer.Start();
	const auto type = packet->PacketType();
	PacketLogger::LogPacket(" <- ", packet);

	if(type == ThingifyPacketType::ThingSessionCreateAck)
	{
		const auto thingSessionCreateAck = static_cast<ThingSessionCreateAckPacket*>(packet);
		_inTopic = thingSessionCreateAck->ClientInServerOutTopic;
		_packetSender.SetOutTopic(thingSessionCreateAck->ClientOutServerInTopic);

		auto sessionCreateResult = thingSessionCreateAck->Type;
		if (sessionCreateResult != LoginResponseType::Success)
		{			
			switch (sessionCreateResult)
			{		
			case LoginResponseType::InvalidCredentials:
				SetError(ThingError::InvalidToken);
				break;
			case LoginResponseType::ProtocolViolation:
				SetError(ThingError::ProtocolViolation);
				break;
			case LoginResponseType::Error:
			default:
				SetError(ThingError::OtherLoginError);
				break;
			}
			DisconnectMqtt();
			return;
		}
		FixedString64 inTopicToSubscribe = _inTopic;
		inTopicToSubscribe.append("/#");
		_mqtt.subscribe(inTopicToSubscribe.c_str(), 1);

		auto packet = new ClientReceivedSessionCreateAckPacket();
		if (!SendAndDeletePacket(packet))
		{
			return;
		}
		_connectTime = millis();
		SetState(ThingState::Online);
	}

	if (type == ThingifyPacketType::UpdateNodesFromClient)
	{
		const auto updateNodesPacket = static_cast<UpdateNodesFromClientPacket*>(packet);

		for (auto & update : updateNodesPacket->Updates)
		{
			auto matchingNode = FindNode(update.nodeId.NodeName.c_str());
		
			if (matchingNode->isNull())
			{
				_logger.warn(L("Failed to find node '%s'"), update.nodeId.NodeName.c_str());
				continue;
			}

			NodeUpdateResultItem updateResultItem;
			updateResultItem.nodeId = update.nodeId;
			updateResultItem.UpdateResult.ResultType = DeviceNodeUpdateResultType::Success;		

			if(_updateResults.size() > 20)
			{
				_logger.warn(L("Update results overflow, cached more than 20"));
			}
			else
			{
				bool existing = false;
				for (auto& updateResult : _updateResults)
				{
					if(strcmp(updateResult.nodeId.NodeName.c_str(), matchingNode->name()) == 0)
					{
						NodeId idFromMatchingNode;
						idFromMatchingNode.NodeName = matchingNode->name();
						idFromMatchingNode.DeviceId = _settings.Token;
						updateResult.nodeId = idFromMatchingNode;
						existing = true;
						_logger.info(L("Update result for node '%s' is already queued"), matchingNode->name());
					}
				}
				if (!existing)
				{
					_updateResults.add(updateResultItem);
				}
			}			
			auto requestedValueStr = update.nodeValue.toString();

			_logger.info(L("Client requested update of node %s to '%s'"), matchingNode->name(), requestedValueStr.c_str());

			matchingNode->SetValueFromServer(update.nodeValue);
			OnNodeValueChanedByExternal(*matchingNode);
		}
		for (int i =0; i < updateNodesPacket->FunctionExecutionRequests.size(); i++)
		{
			auto & functionExecutionRequest = updateNodesPacket->FunctionExecutionRequests[i];
			auto matchingNode = FindNode(functionExecutionRequest.nodeId.NodeName.c_str());

			if (matchingNode->isNull())
			{
				_logger.warn(L("Failed to find node '%s'"), functionExecutionRequest.nodeId.NodeName.c_str());
				continue;
			}
			
			_logger.info(L("Received request to execute %s"), functionExecutionRequest.nodeId.NodeName.c_str());
			auto returnValue = matchingNode->ExecutionCallback(matchingNode->_context, functionExecutionRequest.Arguments);
			OnFunctionExecutedByExternal(*matchingNode);

			FunctionExecutionResponseItem functionResponse(functionExecutionRequest.nodeId, true, functionExecutionRequest.FunctionId, returnValue);		

			_functionExecutionResults.add(functionResponse);
		}
	}
	if (type == ThingifyPacketType::UpdateFirmwareBeginToThing)
	{
		const auto updateFirmwareBegin = static_cast<UpdateFirmwareBeginToThingPacket*>(packet);
		_firmwareUpdateService.HandleUpdateFirmwareBegin(updateFirmwareBegin);
	}
	if (type == ThingifyPacketType::UpdateFirmwareCommitToThing)
	{
		const auto updateFirmwareCommit = static_cast<UpdateFirmwareCommitToThingPacket*>(packet);
		_firmwareUpdateService.HandleFirmwareCommitPacket(updateFirmwareCommit);
	}

}

void Thingify::onMqttMessage(char* topic, 	char* payloadData, AsyncMqttClientMessageProperties properties, 
	size_t len, size_t index, size_t payloadLength)
{
	_logger.debug(LogComponent::Mqtt, L("message received on topic %s, len=%d, index=%d, payloadLength=%d"), topic, len, index,  payloadLength);

	if (HasTerminalState(F("onMqttMessage")))
	{
		return;
	}

	if(topic == nullptr)
	{
		_logger.err(L("Received null topic"));
		return;
	}
	if(payloadData == nullptr)
	{
		_logger.err(L("Received null payload"));
		return;
	}



	if(strcmp(topic, ThingifyConstants::ServerStatusTopic) == 0 && payloadLength > 0)
	{
		const bool isAvailable = payloadData[0] == '1';

		_logger.info((L(" ### Server availability changed: %d")), isAvailable);
		if(isAvailable == false)
		{
			SetState(ThingState::ServiceUnavailable);
		}
		else
		{
			SetState(ThingState::Authenticating);
			Authenticate();
		}
	}
	uint32_t correlationId;
	uint64_t dataIndex;
	if(FirmwareUpdateService::ParseFirmwareTopic(topic, correlationId, dataIndex))
	{		
		_firmwareUpdateService.HandleFirmwareData(correlationId, dataIndex, payloadData, index, len, payloadLength);		
	}
	else if(strcmp(topic, _inTopic.c_str()) == 0)
	{
		auto payload = new FixedString1024;
		payload->append(payloadData, payloadLength);
		const auto packet = Serializer::DeserializePacket(*payload);
		delete payload;

		if(packet == nullptr)
		{
			_logger.err(L("Failed to deserialize incoming packet"));
			return;
		}	
		_incomingPackets++;
		HandlePacket(packet);
		delete packet;		
	}

}

void Thingify::onMqttPublishAcknowlendged(uint16_t packetId)
{
	_logger.debug(LogComponent::Mqtt, L("Publish ACK received: packetId = %d"), packetId);
}


void Thingify::SetState(ThingState newState)
{
	if (_currentState == newState)
	{
		return;
	}
	_stateChangeTimer.Start();
	_logger.info(L("   STATE CHANGE: %s -> %s"),
		ThingifyUtils::ThingStateToStr(_currentState),
		ThingifyUtils::ThingStateToStr(newState));

	_currentState = newState;
	if (OnStateChanged != nullptr)
	{
		OnStateChanged(_currentState);
	}	
}

ThingState Thingify::GetCurrentState() const
{
	return _currentState;
}

const char* Thingify::GetServerName() const
{
	return _settings.ApiServer.c_str();
}

int Thingify::GetReconnectCount() const
{
	return _reconnectCount;
}

uint64_t Thingify::GetMillisecondsSinceConnect()
{
	return _stateChangeTimer.ElapsedMs();
}

void Thingify::AddModule(IModule * module)
{
	_modules.push_back(module);
}

void Thingify::AddDiagnostics(int updateInteval)
{
	auto diagnostics = new DiagnosticsModule(*this);
	diagnostics->UpdateIntervalInMs = updateInteval;
	AddModule(diagnostics);
}

void Thingify::AddStatusLed(int ledPin, bool isLedInverted)
{
	auto statusLedModule = new StatusLedModule(*this, ledPin, isLedInverted);
	AddModule(statusLedModule);
}

void Thingify::SendNodeValues()
{
	auto updatedNodes = GetUpdatedNodes();	

	if (updatedNodes.empty() && _updateResults.empty() && _functionExecutionResults.empty())
	{
		return;
	}

	static SoftTimer maxUpdateIntervalTimer(ThingifyConstants::MinimumIntervalBetweenUpdates);
	if (_updateResults.empty() && _functionExecutionResults.empty())
	{
		if (!maxUpdateIntervalTimer.IsElapsed())
		{
			return;
		}
	}

	LogUpdatedNodes(updatedNodes);	

	auto updateNodesPacket = new UpdateNodesPacket();
	updateNodesPacket->PacketId = _outgoingPacketId++;
	updateNodesPacket->UpdateResults = _updateResults;
	updateNodesPacket->FunctionExecutionResponses = _functionExecutionResults;

	for (auto& node : updatedNodes)
	{
		NodeId nodeId;
		nodeId.DeviceId = _settings.Token;
		nodeId.NodeName = node->name();

		ValueUpdateItem updateItem(nodeId, node->Value);
		updateNodesPacket->UpdatedNodes.add(updateItem);
	}

	for (auto & node : updatedNodes)
	{
		node->_wasUpdated = false;
	}
	_updateResults.clear();
	_functionExecutionResults.clear();

	SendAndDeletePacket(updateNodesPacket);
}

bool Thingify::HasTerminalState(const __FlashStringHelper* eventType)
{
	if (_currentState == ThingState::Error || _currentState == ThingState::Disabled)
	{
		_logger.warn(L("%s ignored, state = %s"), eventType, ThingifyUtils::ThingStateToStr(_currentState));
		return true;
	}
	return false;
}

void Thingify::OnFunctionExecutedByExternal(Node & node)
{
}
void Thingify::OnNodeValueChanedByExternal(Node & node)
{
}

void Thingify::RestartNetwork()
{
	_logger.warn(L("RestartNetwork"));
	DisconnectMqtt();
	SetState(ThingState::Disabled);
	StopNetwork();
	StartNetwork();
	SetState(ThingState::SearchingForNetwork);
}
void Thingify::HandleWatchdog()
{
	if (!WatchdogEnabled)
	{
		return;
	}

	_stateLoopDetector.FeedState((int)_currentState);

	if(_stateLoopDetector.LoopCount() > 20)
	{
		_logger.err(L("Device stuck in state loop state1: %s, state2: %s"), 
			ThingifyUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State1()),
			ThingifyUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State2()));

		FixedString64 stateLoopString;
			stateLoopString.appendFormat(F("StateLoop:%s:%s"),			
				ThingifyUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State1()),
				ThingifyUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State2()));
				_stateLoopDetector.Reset();
		RestartNetwork();
		return;
	}

	const int MaxSecondsForState = 60;
	if (_stateChangeTimer.ElapsedSeconds() >= MaxSecondsForState)
	{
		if (_currentState == ThingState::Authenticating ||
			_currentState == ThingState::ConnectingToMqtt ||
			_currentState == ThingState::ConnectingToNetwork ||
			_currentState == ThingState::SearchingForNetwork ||
			_currentState == ThingState::ServiceUnavailable)
		{
			_logger.err(L("Device stuck in %s state for 60s"), ThingifyUtils::ThingStateToStr(_currentState));

			FixedString64 stateStuckString;
			stateStuckString.appendFormat(F("StateStuck:%s:%ds"),
				ThingifyUtils::ThingStateToShortStr(_currentState),
				MaxSecondsForState);

			RestartNetwork();
			return;
		}
	}

	if (_currentState == ThingState::Online)
	{
		if (_lastPacketReceivedTimer.ElapsedSeconds() > 180)
		{
			_logger.err(L("Thing didn't receive any data for 180s"));
			_lastPacketReceivedTimer.Start(); // hack
			RestartNetwork();
			return;
		}
	}
	
	if (_currentState == ThingState::Error)
	{
		if (_stateChangeTimer.ElapsedSeconds() > 5)
		{
			ThingifyUtils::WriteRestartReason(_errorStr);
			ThingifyUtils::RestartDevice();
		}
	}
}

void Thingify::CheckErrors()
{
	if (FixedString_OverflowDetected)
	{
		SetError(ThingError::StringOverflow);
	}

	if (FixedList_OverflowDetected)
	{
		SetError(ThingError::ListOverflow);
	}

	if (ESP.getFreeHeap() < 1000)
	{
		SetError(ThingError::LowFreeHeap);
	}
#ifdef ESP8266
	if (ESP.getFreeContStack() < 300)
	{
		SetError(ThingError::LowFreeStack);
	}
#endif // ESP8266
}

