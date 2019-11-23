#ifndef _HEARTBEAT_PACKET_H
#define _HEARTBEAT_PACKET_H
#include "PacketBase.h"

class HeartbeatPacket : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::Heartbeat;
	}
	int HeartbeatNumber;
};

#endif

