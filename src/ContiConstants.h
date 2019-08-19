#pragma once

class ContiConstants
{
public:
	static constexpr char* LoginRequestTopicPrefix = "conti/d/req/";
	static constexpr char* LoginResponseTopicPrefix = "conti/d/res/";
	static constexpr char* ServerStatusTopic = "conti/server/status";
	static constexpr char* LastWillTopicPrefix = "conti/client/lw/";

	static const int MinimumIntervalBetweenUpdates = 1000; //in ms

	static const int MaxOutgoingUpdatesCount = 25;
	static const int MaxUpdatesFromClient = 5;
	static const int MaxUpdateResults = 5;

	static const int MaxFunctionExecutionRequests = 5;
	static const int MaxFunctionArguments = 3;
	static const int MaxFunctionErrorResponseBytes = 40;


	static const int MaxStringValueSize = 100;
	static const int MaxDeviceNameLength = 50;
	static const int MaxNodeNameLength = 50;

	static const int MaxErrorStringLength = 50;

	static constexpr char *DefaultServer = "conti.ml";
	static constexpr int DefaultPort = 1883;
	static constexpr int DefaultSerialBaudrate = 115200;
	static constexpr int MqttKeepAliveInSeconds = 60;

	static constexpr int MqttConnectingTimeout = 30000;
	static constexpr int AuthenticatingTimeout = 8000;
};
