#ifndef _UPDATE_NODES_FROM_CLIENT_PACKET_H
#define _UPDATE_NODES_FROM_CLIENT_PACKET_H

#include "NodeUpdate.h"
#include "FunctionExecutionRequestItem.h"
#include "PacketBase.h"
#include "Lib/FixedList/FixedList.h"
#include "ThingifyConstants.h"

class UpdateNodesFromClientPacket : public PacketBase
{
public:
	ThingifyPacketType PacketType() override
	{
		return ThingifyPacketType::UpdateNodesFromClient;
	}
	int PacketId;
	FixedList<NodeUpdate, ThingifyConstants::MaxUpdatesFromClient> Updates;
	FixedList<FunctionExecutionRequestItem,ThingifyConstants::MaxFunctionExecutionRequests> FunctionExecutionRequests;
};

#endif

