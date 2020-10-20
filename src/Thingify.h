#ifndef _THINGIFY_H
#define _THINGIFY_H

#include <vector>
#include "Lib/AsyncMqtt/AsyncMqttClient.h"
#include <FixedString.h>
#include "Node/Node.h"
#include "ThingState.h"
#include "Settings/SettingsStorage.h"
#include "Serialization/Serializer.h"
#include "ThingifyUtils.h"
#include "IModule.h"
#include "Logging/Logger.h"
#include "Helpers/PacketLogger.h"
#include "Helpers/StateLoopDetector.h"
#include "ThingError.h"
#include "Helpers/LoopWatchdog.h"
#include "PacketSender.h"
#include "FirmwareUpdateService.h"
#include "Node/NodeCollection.h"

class IModule;

class Thingify : public NodeCollection
{
private:
	AsyncMqttClient _mqtt;
	void onMqttConnected(bool sessionPresent);
	void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);	
	void onMqttSubscribe(uint16_t packetId);
	void HandlePacket(PacketBase * packet);
	void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t tota);
	void onMqttPublishAcknowlendged(uint16_t packetId);
	
	void SubscribeToEvents();
	void ConnectToServer();
	void SetState(ThingState newState);
	bool HasTerminalState(const __FlashStringHelper* eventType);
	void RestartNetwork();

	std::vector<IModule*> _modules;

	FixedString64 _inTopic;
	FixedString64 _lastWillTopic;
	ElapsedTimer _lastPacketReceivedTimer;

	void SendNodeValues();
	bool SendAndDeletePacket(PacketBase *packet);
	void HandleWatchdog();
	void DisconnectMqtt();
	ThingState _currentState;
	int _reconnectCount;
	uint16_t _incomingPackets;
	int _publishedNodeCount;
	int _outgoingPacketId;
	uint64_t _disconnectedTimer;
	ElapsedTimer _stateChangeTimer;
	uint64_t _connectTime = 0;
	FixedList<NodeUpdateResultItem, ThingifyConstants::MaxUpdateResults> _updateResults;
	FixedList<FunctionExecutionResponseItem, ThingifyConstants::MaxFunctionExecutionRequests> _functionExecutionResults;
	uint16_t _valueSendInterval;
	ThingError _errorType;
	FixedString64 _errorStr;
	FixedString64 _restartReason;
	LoopWatchdog _loopWatchdog;
	LoopStateDetector _stateLoopDetector;
	PacketSender _packetSender;
	void CheckErrors();	
protected:
	void SetError(const char* errorStr);
	void SetError(const __FlashStringHelper* errorStr);
	void SetError(ThingError error, const char* errorStr = nullptr);

	void OnNetworkConnecting();
	void OnNetworkConnected();
	void OnNetworkDisconnected();
	void Authenticate();

	virtual void OnFunctionExecutedByExternal(Node& node);
	virtual void OnNodeValueChanedByExternal(Node& node);
	virtual bool IsNetworkConnected() = 0;
	virtual void StopNetwork() = 0;
	virtual void StartNetwork() = 0;
	virtual void StartZeroConfiguration() = 0;
	virtual bool IsZeroConfigurationReady() = 0;
	virtual void OnConfigurationLoaded();
	ThingSettings _settings;
	bool _isUsingManualConfiguration = false;
	Logger& _logger;
	const char* _deviceName;	
	FirmwareUpdateService _firmwareUpdateService;
	SettingsStorage _settingsStorage;
	virtual uint64_t WatchdogTimeoutInMs() = 0;
	void StartInternal();
public:
	Thingify(const char *deviceName, IAsyncClient& client);
	void SetToken(const char* token);
	virtual void Start();
	void Stop();
	void ResetConfiguration();	
	virtual void Loop();
	bool WatchdogEnabled;
	virtual FixedStringBase& GetNetworkName() = 0;

	uint16_t GetConnectTime()
	{
		return _connectTime;
	}

	uint16_t GetIncomingPackets()
	{
		return _incomingPackets;
	}

	uint16_t GetOutgoingPackets()
	{
		return _packetSender.GetSentPacketCount();
	}

	ThingError GetErrorType()
	{
		return _errorType;
	}
	const char* GetErrorStr()
	{
		return _errorStr.c_str();
	}
	const char* GetRestartReason()
	{
		return _restartReason.c_str();
	}
	const char* GetServerName() const;
	int GetReconnectCount() const;
	uint64_t GetMillisecondsSinceConnect();

	void SetValueSendInterval(uint16_t value)
	{
		_valueSendInterval = value;
	}


	FirmwareUpdateService& UpdateService() 
	{
		return _firmwareUpdateService;
	}
	ThingState GetCurrentState() const;
	std::function<void(ThingState state)> OnStateChanged;
	
	void AddModule(IModule *module);
	void AddDiagnostics(int updateInteval = 10000);
	void AddStatusLed(int ledPin, bool isLedInverted = false);

	virtual ~Thingify() = default;
};

#endif
