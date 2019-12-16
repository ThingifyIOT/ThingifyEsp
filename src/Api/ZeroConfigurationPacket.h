#ifndef _ZERO_CONFIGURATION_PACKET_H
#define _ZERO_CONFIGURATION_PACKET_H

#include <FixedString.h>
#include "PacketBase.h"


class WifiNetworkPacket
{
    public:
        FixedString32 Name;
        FixedString64 Password;
};

class ZeroConfigurationPacket: public PacketBase
{
public:
	FixedString32 Token;
    FixedString32 ApiAddress;
	std::vector<WifiNetworkPacket*> WifiNetworks;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ZeroConfigurationPacket;
	}
};
#endif