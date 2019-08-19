#ifndef _UPDATE_NODES_FROM_CLIENT_PACKET_H
#define _UPDATE_NODES_FROM_CLIENT_PACKET_H

#include "NodeUpdate.h"
#include "FunctionExecutionRequestItem.h"
#include "PacketBase.h"
#include "Lib/FixedList/FixedList.h"
#include "ContiConstants.h"

class UpdateNodesFromClientPacket : public PacketBase
{
public:
	ContiPacketType PacketType() override
	{
		return ContiPacketType::UpdateNodesFromClient;
	}
	int PacketId;
	FixedList<NodeUpdate, ContiConstants::MaxUpdatesFromClient> Updates;
	FixedList<FunctionExecutionRequestItem,ContiConstants::MaxFunctionExecutionRequests> FunctionExecutionRequests;
};

#endif

