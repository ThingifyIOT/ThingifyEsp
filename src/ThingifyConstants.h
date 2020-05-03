#pragma once

class ThingifyConstants
{
public:
	static constexpr const  char* LoginRequestTopicPrefix = "conti/d/req/";
	static constexpr const  char* LoginResponseTopicPrefix = "conti/d/res/";
	static constexpr const  char* ServerStatusTopic = "conti/server/status";
	static constexpr const  char* LastWillTopicPrefix = "conti/client/lw/";

	static const int MinimumIntervalBetweenUpdates = 1000; //in ms

	static const int MaxOutgoingUpdatesCount = 25;
	static const int MaxUpdatesFromClient = 5;
	static const int MaxUpdateResults = 5;

	static const int MaxFunctionExecutionRequests = 5;
	static const int MaxFunctionArguments = 3;
	static const int MaxFunctionErrorResponseBytes = 64;


	static const int MaxStringValueSize = 128;
	static const int MaxDeviceNameLength = 64;
	static const int MaxNodeNameLength = 64;

	static const int MaxErrorStringLength = 64;
	static constexpr int DefaultSerialBaudrate = 115200;
	static constexpr int MqttKeepAliveInSeconds = 60;

	static constexpr int MqttConnectingTimeout = 30000;
	static constexpr int AuthenticatingTimeout = 8000;
};
