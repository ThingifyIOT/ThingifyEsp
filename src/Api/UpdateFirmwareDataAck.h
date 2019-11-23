#ifndef _UPDATE_FIRMWARE_DATA_ACK_H
#define _UPDATE_FIRMWARE_DATA_ACK_H

#include "PacketBase.h"
#include <inttypes.h>
#include <FixedString.h>

class UpdateFirmwareDataAck : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::UpdateFirmwareDataAck;
	}
	uint16_t CorrelationId;
	bool IsSuccess;
	FixedString100 ErrorString;
	uint32_t MaxChunkSize;
};

#endif