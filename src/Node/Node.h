#ifndef _NODE_H
#define _NODE_H


#include "Value/NodeValue.h"
#include "Value/ThingifyUnit.h"

#include "NodeType.h"
#include "NodeKind.h"

#include <FixedString.h>
#include "Lib/FixedList/FixedList.h"

#include "ThingifyConstants.h"
#include "Logging/ContiLogger.h"

class Node;

typedef FixedList<NodeValue, ThingifyConstants::MaxFunctionArguments> FunctionArguments;
typedef NodeValue(*FunctionExecutionCallback)(void* context, FunctionArguments& args);
typedef bool(*ValueChangeHandler)(void* context, Node *node);

extern int ValueinstanceCount;

class Node
{
	friend class Thingify;
	friend class Serializer;
	void *_context;
	FixedString<ThingifyConstants::MaxDeviceNameLength> _name;
	uint16_t _integerId;
	ThingifyUnit _unit;
	bool _isReadOnly;
	ContiLogger& _logger;
	NodeKind _kind;
	int _rangeMin;
	int _rangeMax;
	int _rangeStep;
public:
	Node(NodeType type, ContiType valueType, const char* name, int integerId, ThingifyUnit unitType);
	void SetRangeAttributes(int min, int max, int step)
	{
		_rangeMin = min;
		_rangeMax = max;
		_rangeStep = step;
	}
	const char* name() const;
	const ThingifyUnit unit() const;
	const NodeKind kind() const;
	uint16_t integerId() const;
	bool isReadOnly() const;

	NodeType Type;
	NodeValue Value;
	bool _wasUpdated;
	bool IsHidden;
	void HandleUpdate();
	Node* OnChanged(void *context, ValueChangeHandler handler);
	Node* OnChanged(ValueChangeHandler handler);
	ValueChangeHandler ValueChanged;
	void SetValue(uint32_t value);
	void SetValue(int value);
	void SetValue(NodeValue newValue);
	void SetValueFromServer(NodeValue& newValue);
	Node* SetKind(NodeKind kind);
	FunctionExecutionCallback ExecutionCallback;
};

#endif
