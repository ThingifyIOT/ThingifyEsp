#ifndef _THING_SESSION_CREATE_ACK_PACKET_H
#define _THING_SESSION_CREATE_ACK_PACKET_H

#include <FixedString.h>
#include "LoginResponseType.h"

class ThingSessionCreateAckPacket: public PacketBase
{
public:
	LoginResponseType Type;
	FixedString64 ClientInServerOutTopic;
	FixedString64 ClientOutServerInTopic;
	int64_t CurrentTime;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ThingSessionCreateAck;
	}

};

#endif
