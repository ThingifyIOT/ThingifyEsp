#ifndef _CLIENT_RECEIVED_SESSION_CREATE_ACK_PACKET_H
#define _CLIENT_RECEIVED_SESSION_CREATE_ACK_PACKET_H

#include "PacketBase.h"

class ClientReceivedSessionCreateAckPacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::ClientReceivedCreateSessionAck;
	}
	int PacketId;
};

#endif