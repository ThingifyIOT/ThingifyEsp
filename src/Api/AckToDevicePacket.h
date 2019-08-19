#ifndef _ACK_TO_DEVICE_PACKET_H
#define _ACK_TO_DEVICE_PACKET_H

#include "PacketBase.h"

class AckToDevicePacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::AckToDevice;
	}
	int PacketId;
};

#endif

