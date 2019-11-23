#ifndef _ACK_TO_DEVICE_PACKET_H
#define _ACK_TO_DEVICE_PACKET_H

#include "PacketBase.h"

class AckToDevicePacket : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::AckToDevice;
	}
	int PacketId;
};

#endif

