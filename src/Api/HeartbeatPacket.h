#ifndef _HEARTBEAT_PACKET_H
#define _HEARTBEAT_PACKET_H
#include "PacketBase.h"

class HeartbeatPacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::Heartbeat;
	}
	int HeartbeatNumber;
};

#endif

