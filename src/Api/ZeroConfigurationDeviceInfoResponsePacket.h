#ifndef _ZERO_CONFIGURATION_DEVICE_INFO_RESPONSE_PACKET_H
#define _ZERO_CONFIGURATION_DEVICE_INFO_RESPONSE_PACKET_H

#include "PacketBase.h"
#include <vector>
#include <FixedString.h>

class ZeroConfigurationDeviceInfoResponsePacket: public PacketBase
{
public:
	FixedString128 DefaultName;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ZeroConfigurationDeviceInfoResponsePacket;
	}
};
#endif