#ifndef _THING_SESSION_CREATE_PACKET_H
#define _THING_SESSION_CREATE_PACKET_H

#include <FixedString.h>
#include "PacketBase.h"
#include "Node/Node.h"
#include <vector>

class ThingSessionCreatePacket: public PacketBase
{
public:
	FixedString64 ClientId;
	FixedString128 DeviceName;
	FixedString32 LoginToken;
	FixedString16 FirmwareVersion;
	std::vector<Node*> Nodes;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ThingSessionCreate;
	}
};
#endif
