#ifndef _PACKET_SENDER_H
#define _PACKET_SENDER_H


#include "Lib/AsyncMqtt/AsyncMqttClient.h"
#include "Api/PacketBase.h"
#include "Logging/Logger.h"
#include <FixedString.h>
#include <stdint.h>

class PacketSender
{
	Logger& _logger;
	FixedString64 _outTopic;
	AsyncMqttClient& _mqtt;
	uint16_t _sentPacketCount;
public:

	PacketSender(AsyncMqttClient& mqtt);

	bool SendPacket(PacketBase* packet);
	void SetOutTopic(FixedStringBase& outTopic);
	uint16_t GetSentPacketCount();
};


#endif