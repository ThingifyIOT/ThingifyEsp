#ifndef _PACKET_BASE
#define _PACKET_BASE

#include "ContiPacketType.h"

class PacketBase
{
public:
	virtual ~PacketBase() = default;
	virtual ContiPacketType PacketType() =0;
};

#endif

