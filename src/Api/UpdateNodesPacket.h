#ifndef _UPDATE_NODES_PACKET_H
#define _UPDATE_NODES_PACKET_H

#include "Node/Node.h"
#include "DeviceNodeUpdateResult.h"
#include "FunctionExecutionResponseItem.h"
#include "Lib/FixedList/FixedList.h"
#include "Node/NodeId.h"
#include "PacketBase.h"

class NodeUpdateResultItem
{
public:
	NodeId nodeId;
	DeviceNodeUpdateResult UpdateResult;
};

class ValueUpdateItem
{
public:
	ValueUpdateItem(NodeId nodeId, NodeValue nodeValue)
		:nodeId(nodeId), Value(nodeValue)
	{

	}
	NodeId nodeId;
	NodeValue Value;
};

class UpdateNodesPacket: public PacketBase
{
public:
	int PacketId;
	FixedList<NodeUpdateResultItem, ContiConstants::MaxUpdateResults> UpdateResults;
	FixedList<FunctionExecutionResponseItem, ContiConstants::MaxFunctionExecutionRequests> FunctionExecutionResponses;
	FixedList<ValueUpdateItem, ContiConstants::MaxOutgoingUpdatesCount> UpdatedNodes;

	ContiPacketType PacketType() override
	{
		return ContiPacketType::UpdateNodes;
	}
};

#endif

