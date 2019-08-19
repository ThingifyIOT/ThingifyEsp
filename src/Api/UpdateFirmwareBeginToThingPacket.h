#ifndef _UPDATE_FIRMWARE_BEGIN_TO_THING_PACKET_H
#define _UPDATE_FIRMWARE_BEGIN_TO_THING_PACKET_H

#include "PacketBase.h"
#include <inttypes.h>
#include <FixedString.h>

class UpdateFirmwareBeginToThingPacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::UpdateFirmwareBeginToThing;
	}
	uint32_t CorrelationId;
	uint64_t FirmwareSize;
	FixedString50 FirmwareMd5;
};

#endif