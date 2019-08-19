#ifndef  _FUNCTION_EXECUTION_REQUEST_ITEM_H
#define _FUNCTION_EXECUTION_REQUEST_ITEM_H

#include "Lib/FixedList/FixedList.h"
#include "Value/NodeValue.h"
#include "Node/Node.h"
#include "Node/NodeId.h"

class FunctionExecutionRequestItem
{
public:
	NodeId nodeId;
	uint64_t FunctionId;
	FunctionArguments Arguments;
};

#endif

