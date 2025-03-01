#ifndef _ZERO_CONFIGURATION_DEVICE_INFO_REQUEST_PACKET_H
#define _ZERO_CONFIGURATION_DEVICE_INFO_REQUEST_PACKET_H

#include "PacketBase.h"
#include <vector>

class ZeroConfigurationDeviceInfoRequestPacket: public PacketBase
{
public:

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ZeroConfigurationDeviceInfoRequestPacket;
	}
};
#endif