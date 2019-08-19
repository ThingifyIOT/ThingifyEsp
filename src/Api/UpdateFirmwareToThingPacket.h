#ifndef _UPDATE_FIRMWARE_TO_THING_PACKET_H
#define _UPDATE_FIRMWARE_TO_THING_PACKET_H

#include "PacketBase.h"
#include <inttypes.h>

class UpdateFirmwareCommitToThingPacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::UpdateFirmwareCommitToThing;
	}
	uint32_t CorrelationId;
};

#endif