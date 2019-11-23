#ifndef _PACKET_BASE
#define _PACKET_BASE

#include "ThingifyPacketType.h"

class PacketBase
{
public:
	virtual ~PacketBase() = default;
	virtual ThingifyPacketType PacketType() =0;
};

#endif

