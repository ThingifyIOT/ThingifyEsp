#ifndef _NODE_H
#define _NODE_H


#include "Value/NodeValue.h"
#include "Value/ThingifyUnit.h"

#include "NodeType.h"
#include "NodeKind.h"

#include <FixedString.h>
#include "Lib/FixedList/FixedList.h"

#include "ThingifyConstants.h"
#include "Logging/Logger.h"

class Node;

typedef FixedList<NodeValue, ThingifyConstants::MaxFunctionArguments> FunctionArguments;
typedef NodeValue(*FunctionExecutionCallback)(void* context, FunctionArguments& args);
typedef bool(*ValueChangeHandler)(void* context, Node *node);

extern int ValueinstanceCount;

class Node
{
	friend class Serializer;
	friend class Thingify;
	friend class NodeCollection;
	void *_context;
	FixedString<ThingifyConstants::MaxDeviceNameLength> _name;
	uint16_t _integerId;
	ThingifyUnit _unit;
	bool _isReadOnly;
	Logger& _logger;
	NodeKind _kind;
	int _rangeMin;
	int _rangeMax;
	int _rangeStep;
public:
	Node(NodeType type, ValueType valueType, const char* name, int integerId, ThingifyUnit unitType);
	void SetRangeAttributes(int min, int max, int step)
	{
		_rangeMin = min;
		_rangeMax = max;
		_rangeStep = step;
	}
	const char* name() const;
	const ThingifyUnit unit() const;
	const NodeKind kind() const;
	const bool isNull() const;
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
    Node* Set(bool value);
	Node* Set(int value);
    Node* Set(float value);
	Node* SetValue(NodeValue newValue);
	void SetValueFromServer(NodeValue& newValue);
	Node* SetKind(NodeKind kind);
	FunctionExecutionCallback ExecutionCallback;
};

#endif
