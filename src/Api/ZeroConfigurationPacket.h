#ifndef _ZERO_CONFIGURATION_PACKET_H
#define _ZERO_CONFIGURATION_PACKET_H

#include <FixedString.h>
#include "PacketBase.h"


class WifiNetworkPacket
{
    public:
        FixedString50 Name;
        FixedString50 Password;
};

class ZeroConfigurationPacket: public PacketBase
{
public:
	FixedString50 Token;
    FixedString50 ApiAddress;
	std::vector<WifiNetworkPacket*> WifiNetworks;

	ContiPacketType PacketType() override
	{
		return ContiPacketType::ZeroConfigurationPacket;
	}
};
#endif