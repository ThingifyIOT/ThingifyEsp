#ifndef _ZERO_CONFIGURATION_PACKET_H
#define _ZERO_CONFIGURATION_PACKET_H

#include <FixedString.h>
#include "PacketBase.h"
#include <vector>

class WifiNetworkPacket
{
    public:
        FixedString32 Name;
        FixedString64 Password;
};

class ZeroConfigurationPacket: public PacketBase
{
public:
	FixedString64 ThingName;
	FixedString32 Token;
    FixedString32 ApiServer;
	int ApiPort;
	std::vector<WifiNetworkPacket*> WifiNetworks;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ZeroConfigurationPacket;
	}
	~ZeroConfigurationPacket()
	{
		for(int i=0; i < WifiNetworks.size(); i++)
		{
			delete WifiNetworks[i];
		}
	}
};
#endif