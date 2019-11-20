#ifndef _CONTI_H
#define _CONTI_H

#include <vector>
#include "Lib/AsyncMqtt/AsyncMqttClient.h"
#include <FixedString.h>
#include "Node/Node.h"
#include "ThingState.h"
#include "Serialization/Serializer.h"
#include "ContiUtils.h"
#include "IModule.h"
#include "Logging/Logger.h"
#include "Helpers/PacketLogger.h"
#include "Helpers/StateLoopDetector.h"
#include "ThingError.h"
#include "Helpers/LoopWatchdog.h"
#include "PacketSender.h"
#include "FirmwareUpdateService.h"

class IModule;

class Thingify
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
	void LogNodes();
	void SetState(ThingState newState);
	bool HasTerminalState(const __FlashStringHelper* eventType);
	std::vector<Node*> GetWorkingNodes();
	std::vector<Node*> GetUpdatedNodes();

	std::vector<Node*> _nodes;
	std::vector<IModule*> _modules;
	FixedString<32> _networkName;

	FixedString50 _inTopic;
	FixedString50 _lastWill;
	ElapsedTimer _lastPacketReceivedTimer;
	
	void LogUpdatedNodes(std::vector<Node*> updatedNodes) const;
	void SendNodeValues();
	bool SendAndDeletePacket(PacketBase *packet);
	void HandleWatchdog();
	void DisconnectMqtt();
	ThingState _currentState;
	int _serverPort;
	int _reconnectCount;
	uint16_t _incomingPackets;
	uint16_t _lastNodeId;
	int _publishedNodeCount;
	int _outgoingPacketId;
	uint64_t _disconnectedTimer;
	ElapsedTimer _stateChangeTimer;
	uint64_t _connectTime = 0;
	FixedList<NodeUpdateResultItem, ThingifyConstants::MaxUpdateResults> _updateResults;
	FixedList<FunctionExecutionResponseItem, ThingifyConstants::MaxFunctionExecutionRequests> _functionExecutionResults;
	uint16_t _valueSendInterval;
	ThingError _errorType;
	FixedString50 _errorStr;
	FixedString50 _restartReason;
	LoopWatchdog _loopWatchdog;
	LoopStateDetector _stateLoopDetector;
	PacketSender _packetSender;
	void CheckErrors();
protected:
	void SetError(const char* errorStr);
	void SetError(const __FlashStringHelper* errorStr);
	void SetError(ThingError error, const char* errorStr = nullptr);

	void OnNetworkConnecting(FixedStringBase& networkName);
	void OnNetworkConnected();
	void OnNetworkDisconnected();
	void Authenticate();

	virtual void OnFunctionExecutedByExternal(Node& node);
	virtual void OnNodeValueChanedByExternal(Node& node);
	virtual bool IsNetworkConnected() = 0;
	Logger& _logger;
	const char* _deviceName;
	const char* _deviceId;
	const char* _serverName;
	
	FirmwareUpdateService _firmwareUpdateService;
	virtual uint64_t WatchdogTimeoutInMs() = 0;
public:
	Thingify(const char *deviceId, const char *deviceName, IAsyncClient& client);
	virtual void Start();
	void Stop();
	virtual void Loop();

	bool WatchdogEnabled;

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

	Node* operator[](const char* node);
	Node* FindNode(const char *nodeName);
	bool RemoveNode(const char *nodeName);	
	Node* AddNode(const char* nodeName, NodeType type, ContiType valueType, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddBoolean(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddString(const char* nodeName, const char *value, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddString(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddRange(const char* nodeName, int min, int max, int step = 1, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddFunction(const char * nodeName, FunctionExecutionCallback callback, void* context = nullptr);
	Node* AddInt(const char * nodeName, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddInt(const char* nodeName, int value, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddFloat(const char* nodeName, float value, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddFloat(const char* nodeName, ThingifyUnit unit = ThingifyUnit::None);
	Node* AddColor(const char* nodeName);
	Node* AddTimeSpan(const char* name);

	const char* GetServerName() const;
	int GetReconnectCount() const;
	uint64_t GetMillisecondsSinceConnect();
	FixedString<50> GetNetworkName()
	{
		return _networkName;
	}

	void AddModule(IModule *module);

	virtual ~Thingify() = default;
};

#endif
