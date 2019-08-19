#ifndef _THING_SESSION_CREATE_ACK_PACKET_H
#define _THING_SESSION_CREATE_ACK_PACKET_H

#include <FixedString.h>
#include "LoginResponseType.h"

class ThingSessionCreateAckPacket: public PacketBase
{
public:
	LoginResponseType Type;
	FixedString<80> ClientInServerOutTopic;
	FixedString<80> ClientOutServerInTopic;
	int64_t CurrentTime;

	ContiPacketType PacketType() override
	{
		return ContiPacketType::ThingSessionCreateAck;
	}

};

#endif
