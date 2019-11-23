#ifndef _UPDATE_FIRMWARE_BEGIN_TO_THING_PACKET_H
#define _UPDATE_FIRMWARE_BEGIN_TO_THING_PACKET_H

#include "PacketBase.h"
#include <inttypes.h>
#include <FixedString.h>

class UpdateFirmwareBeginToThingPacket : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::UpdateFirmwareBeginToThing;
	}
	uint32_t CorrelationId;
	uint64_t FirmwareSize;
	FixedString50 FirmwareMd5;
};

#endif