#include "Node.h"

int ValueinstanceCount = 0;
Node::Node(NodeType type, ValueType valueType, const char* name, int integerId, ThingifyUnit unitType):
_logger(LoggerInstance),
_kind(NodeKind::Normal),
Value(valueType, false),
_wasUpdated(false)
{
	_name = name;
	_unit = unitType;
	_isReadOnly = true;
	this->Type = type;
	ValueChanged = nullptr;
	_context = nullptr;
	IsHidden = false;
	_integerId = integerId;
	ExecutionCallback = nullptr;	
}

const char* Node::name() const
{
	return _name.c_str();
}

const ThingifyUnit Node::unit() const
{
	return _unit;
}
const NodeKind Node::kind() const
{
	return _kind;
}
const bool Node::isNull() const
{
	return Value.Type == ValueType::Null;
}
void Node::HandleUpdate()
{
	_wasUpdated = true;
	if (ValueChanged != nullptr)
		ValueChanged(_context, this);
}

uint16_t Node::integerId() const
{
	return _integerId;
}

bool Node::isReadOnly() const
{
	return _isReadOnly;
}

Node* Node::OnChanged(void *context, ValueChangeHandler handler)
{
	_isReadOnly = false;
	_context = context;
	ValueChanged = handler;
	return this;
}

Node* Node::OnChanged(ValueChangeHandler handler)
{
	return OnChanged(nullptr, handler);
}

Node* Node::Set(bool value)
{
    return SetValue(NodeValue::Boolean(value)); 
}

Node* Node::Set(int value)
{
	return SetValue(NodeValue::Int(value));
}

Node* Node::Set(float value)
{
	return SetValue(NodeValue::Float(value));
}
Node* Node::SetValue(NodeValue newValue)
{
    if(Value.Type != newValue.Type)
    {
        _logger.warn(L("Attempted to set value type %d to node of type %d [ node name = %s]"), newValue.Type, Value.Type, name());
        return this;
    }
	if (Value.IsSameAs(newValue))
	{
		return this;
	}

	Value = newValue;
	_wasUpdated = true;
	HandleUpdate();
    return this;
}

void Node::SetValueFromServer(NodeValue& newValue)
{
	auto valueString = newValue.toString();
	_logger.debug(LogComponent::Node, L("Try set value of node '%s' to '%s'"), name(), valueString.c_str());

	if (Value.IsSameAs(newValue))
	{
		_logger.debug(LogComponent::Node, L("Value of node '%s' same as current one"), name());
		return;
	}
	_logger.debug(LogComponent::Node, L("Value of node '%s' updated to '%s'"), name(), valueString.c_str());

	Value = newValue;
	if (ValueChanged != nullptr)
		ValueChanged(_context, this);
}

Node* Node::SetKind(NodeKind kind)
{
	_kind = kind;
	return this;
}



