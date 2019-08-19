#ifndef _PACKET_SENDER_H
#define _PACKET_SENDER_H


#include "Lib/AsyncMqtt/AsyncMqttClient.h"
#include "Api/PacketBase.h"
#include "Logging/ContiLogger.h"
#include <FixedString.h>
#include <stdint.h>

class PacketSender
{
	ContiLogger& _logger;
	uint16_t _sentPacketCount;
	FixedString50 _outTopic;
	AsyncMqttClient& _mqtt;

public:

	PacketSender(AsyncMqttClient& mqtt);

	bool SendPacket(PacketBase* packet);
	void SetOutTopic(FixedStringBase& outTopic);
	uint16_t GetSentPacketCount();
};


#endif