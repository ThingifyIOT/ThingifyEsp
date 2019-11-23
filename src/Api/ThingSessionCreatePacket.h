#ifndef _THING_SESSION_CREATE_PACKET_H
#define _THING_SESSION_CREATE_PACKET_H

#include <FixedString.h>
#include "PacketBase.h"
#include "Node/Node.h"
#include <vector>

class ThingSessionCreatePacket: public PacketBase
{
public:
	FixedString<40> ClientId;
	FixedString<100> DeviceName;
	FixedString<30> LoginToken;
	FixedString<15> FirmwareVersion;
	std::vector<Node*> Nodes;

	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::ThingSessionCreate;
	}
};
#endif
