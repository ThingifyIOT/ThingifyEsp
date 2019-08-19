#ifndef _NODE_UPDATE_H
#define _NODE_UPDATE_H

#include <FixedString.h>
#include "Value/NodeValue.h"
#include "Node/Node.h"
#include "Node/NodeId.h"

class NodeUpdate
{
public:
	NodeUpdate(NodeId nodeId, NodeValue nodeValue)
		:nodeId(nodeId), nodeValue(nodeValue)
	{
	}
	NodeId nodeId;
	NodeValue nodeValue;
};

#endif


