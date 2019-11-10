#include "Thingify.h"
#include <functional>
#include "Helpers/StringHelper.h"

#include "ThingifyConstants.h"
#include "Api/HeartbeatPacket.h"
#include "Api/DeviceNodeUpdateResult.h"
#include "Api/ClientReceivedSessionCreateAckPacket.h"

#include <EEPROM.h>
using namespace std::placeholders;

Thingify::Thingify(const char *deviceId, const char *deviceName, IAsyncClient& client) :
_mqtt(client),
_currentState(ThingState::Disabled),
_incomingPackets(0),
_valueSendInterval(0),
_errorType(ThingError::NoError),
_packetSender(_mqtt),
_logger(ContiLoggerInstance),
_deviceName(deviceName), 
_deviceId(deviceId),
_firmwareUpdateService(_packetSender)
{
	Serial.println("Conti::Conti");	
	_serverName = ThingifyConstants::DefaultServer;
	_serverPort = ThingifyConstants::DefaultPort;
	_nodes.reserve(32);
	_modules.reserve(16);
	_mqtt.setServer(_serverName, _serverPort);

	_publishedNodeCount = 0;
	_lastNodeId = 0;
	_lastWill.clear();
	_lastWill.appendFormat("%s%s", ThingifyConstants::LastWillTopicPrefix, deviceId);

	_mqtt.
		setKeepAlive(ThingifyConstants::MqttKeepAliveInSeconds).
		setCredentials(_deviceId, "").
		setClientId(_deviceId);

	SubscribeToEvents();	
	_outgoingPacketId = 0;
	_stateChangeTimer.Start();
	WatchdogEnabled = true;

	_logger.info(L("Sizeof ContiValue = %d"), sizeof(NodeValue));
	_logger.info(L("Sizeof UpdateNodesFromClientPacket: %d"), sizeof(UpdateNodesFromClientPacket));
	_logger.info(L("Size of function execution buffer: %d"), sizeof(FixedList<FunctionExecutionResponseItem, ThingifyConstants::MaxFunctionExecutionRequests>));
}

void Thingify::Start()
{
	_logger.info(L("Thing::Start"));
	_errorType = ThingError::NoError;
	EEPROM.begin(100);
	ContiUtils::ReadRestartReason(_restartReason);
	_loopWatchdog.Start(WatchdogTimeoutInMs());
	if (_restartReason.length() > 0)
	{
		_logger.err(L("Restart reason: %s"), _restartReason.c_str());
	}
	ContiUtils::ClearRestartReason();

	_logger.info(L("Module count: %d"), _modules.size());
	for (auto module : _modules)
	{
		_logger.info(L("Initializing module %s"), module->GetName());
		if (!module->Init())
		{
			_logger.err(L("Failed to initialize module '%s': %s"), module->GetName(), module->GetError());
		}
	}

	if (IsNetworkConnected())
	{
		_logger.debug(L("Call ConnectToServer from Conti::Start"));
		ConnectToServer();
	}
	else
	{
		_logger.info(LogComponent::Network, L("wlan not connected, waiting for network"));
		SetState(ThingState::SearchingForNetwork);
	}

	LogNodes();
}

void Thingify::Stop()
{
	_logger.info(L("Thing::Stop"));
	SetState(ThingState::Disabled);
	DisconnectMqtt();
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
	FixedString50 errorTmp;
	
	errorTmp = errorStr;
	SetError(ThingError::Other, errorTmp.c_str());
}

void Thingify::SetError(ThingError error, const char* errorStr)
{
	SetState(ThingState::Error);

	if (_currentState == ThingState::Online)
	{
		_mqtt.publish(_lastWill.c_str(), 2, false, "lw", 2);
		delay(150);
		DisconnectMqtt();
	}
	if (error != ThingError::Other)
	{
		_errorStr = ContiUtils::ThingErrorToStr(error);
	}
	if (errorStr != nullptr)
	{
		_errorStr = errorStr;
	}
	_logger.err(F("Set error: %s"), _errorStr.c_str());

	_errorType = error;
}

void Thingify::OnNetworkConnecting(FixedStringBase& networkName)
{
	_networkName = networkName;
	_logger.info(LogComponent::Network, L("Conti::OnNetworkConnecting: '%s'"), _networkName.c_str());
	if (HasTerminalState(F("OnNetworkConnecting")))
	{
		return;
	}	
	SetState(ThingState::ConnectingToNetwork);
}

void Thingify::OnNetworkConnected()
{
	_logger.info(LogComponent::Network, L("Conti::OnNetworkConnected"));
	if (HasTerminalState(F("OnNetworkConnected")))
	{
		return;
	}
	if (_currentState == ThingState::ConnectingToNetwork ||
		_currentState == ThingState::DisconnectedFromMqtt ||
		_currentState == ThingState::ConnectingToMqtt ||
		_currentState == ThingState::SearchingForNetwork)
	{
		_logger.debug(L("Call ConnectToServer from Conti::OnNetworkConnected"));

		ConnectToServer();
	}
}

void Thingify::OnNetworkDisconnected()
{
	_logger.info(LogComponent::Network, L("Conti::OnNetworkDisconnected"));
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

	auto outTopic = FixedString50(ThingifyConstants::LoginRequestTopicPrefix) + requestId;
	_packetSender.SetOutTopic(outTopic);
	_inTopic = FixedString50(ThingifyConstants::LoginResponseTopicPrefix) + requestId;
	
	const uint16_t packetIdSub = _mqtt.subscribe(_inTopic.c_str(), 2);
	_logger.debug(L("Subscribed to %s, subsId = %d"), _inTopic.c_str(), packetIdSub);

	auto thingSessionCreatePacket = new ThingSessionCreatePacket;
	thingSessionCreatePacket->LoginToken = requestId;
	thingSessionCreatePacket->ClientId = _deviceId;
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
	_logger.debug(L("Last will: %s"), _lastWill.c_str());
	_mqtt.setWill(_lastWill.c_str(), 2, false, "lw", 2);

	SetState(ThingState::ConnectingToMqtt);
	_logger.info(LogComponent::Network, L("Call mqtt.connect()"));

	_mqtt.connect();	
}

void Thingify::DisconnectMqtt()
{
	if (!_mqtt.connected())
	{
		_logger.info(L("DisconnectMqtt: mqtt already disconnected"));
		return;
	}
	_logger.info(L("DisconnectMqtt: call mqtt.disconnect()"));
	_mqtt.disconnect();
}

void Thingify::LogNodes()
{
	_logger.debug(LogComponent::Node, L("Node list: "));
	for(auto& node: _nodes)
	{
		_logger.debug(LogComponent::Node, L(" %s read_only: %d"), node->name(), node->isReadOnly());
	}
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

	if(type == ContiPacketType::ThingSessionCreateAck)
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
		FixedString50 inTopicToSubscribe = _inTopic;
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

	if (type == ContiPacketType::UpdateNodesFromClient)
	{
		const auto updateNodesPacket = static_cast<UpdateNodesFromClientPacket*>(packet);

		for (auto & update : updateNodesPacket->Updates)
		{
			auto matchingNode = FindNode(update.nodeId.NodeName.c_str());
		
			if (matchingNode == nullptr)
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
				_logger.info(L("Added item to update_results"));

				bool existing = false;
				for (auto& updateResult : _updateResults)
				{
					if(strcmp(updateResult.nodeId.NodeName.c_str(), matchingNode->name()) == 0)
					{
						NodeId idFromMatchingNode;
						idFromMatchingNode.NodeName = matchingNode->name();
						idFromMatchingNode.DeviceId = _deviceId;
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

			if (matchingNode == nullptr)
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
	if (type == ContiPacketType::UpdateFirmwareBeginToThing)
	{
		const auto updateFirmwareBegin = static_cast<UpdateFirmwareBeginToThingPacket*>(packet);
		_firmwareUpdateService.HandleUpdateFirmwareBegin(updateFirmwareBegin);
	}
	if (type == ContiPacketType::UpdateFirmwareCommitToThing)
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
		auto payload = new FixedString<1000>;
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
		ContiUtils::ThingStateToStr(_currentState),
		ContiUtils::ThingStateToStr(newState));

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
	return _serverName;
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

Node* Thingify::AddNode(const char* nodeName, NodeType type, ContiType valueType, ThingifyUnit unit)
{
	auto existingNode = FindNode(nodeName);
	if (existingNode != nullptr)
	{
		SetError(F("Attempted to add node with same name"));
		return nullptr;
	}
	const auto node = new Node(type, valueType, nodeName, _lastNodeId, unit);
	_lastNodeId++;
	_nodes.push_back(node);
	return node;
}

Node* Thingify::AddBoolean(const char* nodeName, ThingifyUnit unit)
{
	return AddNode(nodeName, NodeType::BasicValue, ContiType::Bool, unit);
}
Node* Thingify::AddString(const char* nodeName, const char *value, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ContiType::String, unit);
	node->SetValue(NodeValue::String(value));
	return node;
}
Node* Thingify::AddString(const char * nodeName, ThingifyUnit unit)
{
	return AddString(nodeName, "", unit);
}
Node* Thingify::AddInt(const char* nodeName, ThingifyUnit unit)
{
	return AddNode(nodeName, NodeType::BasicValue, ContiType::Int, unit);
}
Node* Thingify::AddInt(const char* nodeName, int value, ThingifyUnit unit)
{	
	auto node = AddNode(nodeName, NodeType::BasicValue, ContiType::Int, unit);
	node->SetValue(NodeValue::Int(value));
	return node;
}
Node* Thingify::AddFloat(const char* nodeName, float value, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ContiType::Float, unit);
	node->SetValue(NodeValue::Float(value));
	return node;
}
Node* Thingify::AddFloat(const char* nodeName, ThingifyUnit unit)
{
	return AddFloat(nodeName, 0.0f, unit);
}
Node* Thingify::AddColor(const char * nodeName)
{
	auto node = AddNode(nodeName, NodeType::BasicValue, ContiType::Float, ThingifyUnit::None);
	node->SetValue(NodeValue::NullColor());
	return node;
}
Node * Thingify::AddTimeSpan(const char *name)
{
	auto node = AddNode(name, NodeType::BasicValue, ContiType::TimeSpan, ThingifyUnit::None);
	node->SetValue(NodeValue::NullColor());
	return node;
}
Node* Thingify::AddRange(const char * nodeName, int min, int max, int step, ThingifyUnit unit)
{
	auto node = AddNode(nodeName, NodeType::Range, ContiType::Int, unit);
	node->SetValue(NodeValue::NullInt());
	node->SetRangeAttributes(min, max, step);
	return node;
}

Node* Thingify::AddFunction(const char * nodeName, FunctionExecutionCallback callback, void* context)
{
	auto node = AddNode(nodeName, NodeType::Function, ContiType::Bool, ThingifyUnit::None);
	node->_context = context;
	node->ExecutionCallback = callback;
	return node;
}

Node * Thingify::operator[](const char * nodeName)
{
	return FindNode(nodeName);
}

Node* Thingify::FindNode(const char* nodeName)
{
	for (auto i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes[i];
		if (strcmp(node->name(), nodeName) == 0)
		{
			return node;
		}
	}
	return nullptr;
}
bool Thingify::RemoveNode(const char *nodeName)
{
	for (auto i = 0; i < _nodes.size(); i++)
	{
		auto node = _nodes[i];
		if (strcmp(node->name(), nodeName) == 0)
		{
			_nodes.erase(_nodes.begin() + i);
			delete node;
			return true;
		}
	}
	return false;
}
std::vector<Node*> Thingify::GetWorkingNodes()
{
	std::vector<Node*> workingNodes;
	workingNodes.reserve(_nodes.size());
	for (auto & node : _nodes)
	{
		if (!node->IsHidden)
		{
			workingNodes.push_back(node);
		}
	}
	return workingNodes;
}

std::vector<Node*> Thingify::GetUpdatedNodes()
{
	std::vector<Node*> updatedNodes;
	updatedNodes.reserve(_nodes.size());
	for (auto & node : _nodes)
	{
		if (node->_wasUpdated && !node->IsHidden)
		{
			updatedNodes.push_back(node);
		}
	}
	return updatedNodes;
}

void Thingify::LogUpdatedNodes(std::vector<Node*> updatedNodes) const
{
	FixedString<50> updateNodesString;

	for (Node* updated_node : updatedNodes)
	{
		updateNodesString.appendFormat("'%s', ", updated_node->name());		
	}

	_logger.debug(L("nodes: [%s] changed value, send updates"), updateNodesString.c_str());
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
		nodeId.DeviceId = _deviceId;
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
		_logger.warn(L("%s ignored, state = %s"), eventType, ContiUtils::ThingStateToStr(_currentState));
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
			ContiUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State1()),
			ContiUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State2()));

		FixedString50 stateLoopString;
			stateLoopString.appendFormat(F("StateLoop:%s:%s"),			
				ContiUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State1()),
				ContiUtils::ThingStateToShortStr((ThingState)_stateLoopDetector.State2()));
				_stateLoopDetector.Reset();

		SetError(stateLoopString.c_str());
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
			_logger.err(L("Device stuck in %s state for 60s"), ContiUtils::ThingStateToStr(_currentState));

			FixedString50 stateStuckString;
			stateStuckString.appendFormat(F("StateStuck:%s:%ds"),
				ContiUtils::ThingStateToShortStr(_currentState),
				MaxSecondsForState);

			SetError(stateStuckString.c_str());
		}
	}

	if (_currentState == ThingState::Error)
	{
		if (_stateChangeTimer.ElapsedSeconds() > 5)
		{
			ContiUtils::WriteRestartReason(_errorStr);
			ContiUtils::RestartDevice();
		}
	}

	if (_currentState == ThingState::Online)
	{
		if (_lastPacketReceivedTimer.ElapsedSeconds() > 120)
		{
			_logger.err(L("Device didn't receive any data for 120s"));
			SetError(F("DataIdle120s"));
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
