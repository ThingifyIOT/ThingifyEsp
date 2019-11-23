#ifndef _CLIENT_RECEIVED_SESSION_CREATE_ACK_PACKET_H
#define _CLIENT_RECEIVED_SESSION_CREATE_ACK_PACKET_H

#include "PacketBase.h"

class ClientReceivedSessionCreateAckPacket : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ClientReceivedCreateSessionAck;
	}
	int PacketId;
};

#endif