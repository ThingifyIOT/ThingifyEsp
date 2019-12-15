#ifndef _ZERO_CONFIGURATION_RESPONSE_PACKET_H
#define _ZERO_CONFIGURATION_RESPONSE_PACKET_H

#include "PacketBase.h"

class ZeroConfigurationResponsePacket: public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ZeroConfigurationResponsePacket;
	}
};
#endif