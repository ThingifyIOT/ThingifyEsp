#ifndef  _FUNCTION_EXECUTION_RESPONSE_ITEM
#define _FUNCTION_EXECUTION_RESPONSE_ITEM

#include "Lib/FixedList/FixedList.h"
#include "Value/NodeValue.h"
#include "ContiConstants.h"
#include "Node/NodeId.h"

class FunctionExecutionResponseItem
{
public:
	FunctionExecutionResponseItem(NodeId nodeId, bool isSuccess, uint64_t functionId, NodeValue returnValue)
		:nodeId(nodeId), IsSuccess(isSuccess), FunctionId(functionId), ReturnValue(returnValue)
	{

	}
	NodeId nodeId;
	bool IsSuccess;
	uint64_t FunctionId;
	NodeValue ReturnValue;
	FixedString<ContiConstants::MaxFunctionErrorResponseBytes> ErrorMessage;
};

#endif

