
#include "Value/NodeValue.h"
#include <string.h>
#include "NodeValue.h"



NodeValue::NodeValue(ValueType type, bool isNull):_isNull(isNull)
{
	Type = type;
	switch(Type)
	{
	case ValueType::Bool:
		_boolValue = false;
		break;
	case ValueType::Int:
		_intValue = 0;
		break;
	case ValueType::String:
		stringValue = "";
		break;
	case ValueType::Float:
		_floatValue = 0.0f;
		break;
	case ValueType::Color:
		_colorValue.R = 0;
		_colorValue.G = 0;
		_colorValue.B = 0;
		_colorValue.A = 0;
		break;
	case ValueType::TimeSpan:
		_timespanValue = 0;
		break;

	case ValueType::Location: break;
	case ValueType::DateTime: break;
	default: ;
	}
}

NodeValue::NodeValue(const NodeValue& other)
{
	Type = other.Type;
	_isNull = other._isNull;

	switch (Type)
	{
	case ValueType::Bool:
		_boolValue = other._boolValue;
		return;
	case ValueType::Int:
		_intValue = other._intValue;
		return;
	case ValueType::String:
		stringValue = other.stringValue;
		return;
	case ValueType::Float:
		_floatValue = other._floatValue;
		return;
	case ValueType::Color:
		_colorValue = other._colorValue;
		return;
	case ValueType::TimeSpan:
		_timespanValue = other._timespanValue;
		return;
	case ValueType::Location:
		break;
	case ValueType::DateTime:
		break;
	}
}

NodeValue & NodeValue::operator=(NodeValue & other)
{
	Type = other.Type;
	_isNull = other._isNull;
	switch (Type)
	{
	case ValueType::Bool:
		_boolValue = other._boolValue;
		break;
	case ValueType::Int:
		_intValue = other._intValue;
		break;
	case ValueType::String:
		stringValue = other.stringValue;
		break;
	case ValueType::Float:
		_floatValue = other._floatValue;
		break;
	case ValueType::Color:
		_colorValue = other._colorValue;
		break;
	case ValueType::TimeSpan:
		_timespanValue = other._timespanValue;
		break;
	case ValueType::Location:
		break;
	case ValueType::DateTime:
		break;
	}
	return *this;
}

NodeValue NodeValue::Boolean(bool boolValue)
{
	NodeValue value(ValueType::Bool);
	value._boolValue = boolValue;
	return value;
}

NodeValue NodeValue::String(const char* stringValue)
{
	NodeValue value(ValueType::String);
	value.stringValue = FixedString<ThingifyConstants::MaxStringValueSize>(stringValue);
	return value;
}

NodeValue NodeValue::Int(int integerValue)
{
	NodeValue value(ValueType::Int);
	value._intValue = integerValue;
	return value;
}

NodeValue NodeValue::Range(int integerValue)
{
	NodeValue value(ValueType::Int);
	value._intValue = integerValue;
	return value;
}

NodeValue NodeValue::Float(float floatValue)
{
	NodeValue value(ValueType::Float);
	value._floatValue = floatValue;
	return value;
}
NodeValue NodeValue::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	NodeValue value(ValueType::Color);
	value._colorValue.R = r;
	value._colorValue.G = g;
	value._colorValue.B = b;
	value._colorValue.A = a;
	return value;
}
NodeValue NodeValue::TimeSpan(uint64_t ms)
{
	NodeValue value(ValueType::TimeSpan);
	value._timespanValue = ms * 10000;
	return value;
}
NodeValue NodeValue::NullString()
{
	NodeValue value(ValueType::String, true);
	return value;
}
NodeValue NodeValue::NullInt()
{
	NodeValue value(ValueType::Int, true);
	return value;
}
NodeValue NodeValue::NullFloat()
{
	NodeValue value(ValueType::Float, true);
	return value;
}
NodeValue NodeValue::NullColor()
{
	NodeValue value(ValueType::Color, true);
	return value;
}
NodeValue NodeValue::NullTimeSpan()
{
	NodeValue value(ValueType::TimeSpan, true);
	return value;
}
bool NodeValue::AsBool() const
{
	return _boolValue;
}

const char* NodeValue::AsString() const
{
	return stringValue.c_str();
}

int NodeValue::AsInt() const
{
	return _intValue;
}

float NodeValue::AsFloat() const
{
	return _floatValue;
}

int NodeValue::RangeValue() const
{
	return _intValue;
}

ContiColor NodeValue::AsColor() const
{
	return _colorValue;
}

bool NodeValue::IsSameAs(NodeValue& other) const
{
	if (other.Type != Type)
	{
		return false;
	}
	if (_isNull && other._isNull)
	{
		return true;
	}
	if (_isNull != other._isNull)
	{
		return false;
	}
	switch (Type)
	{
	case ValueType::Bool:
		return _boolValue == other._boolValue;
	case ValueType::Int:
		return _intValue == other._intValue;
	case ValueType::String:		
		return strcmp(stringValue.c_str(), other.stringValue.c_str()) == 0;
	case ValueType::Float:
		return _floatValue == other._floatValue;	
	default:
		return false;
	}
}

bool NodeValue::IsNull()
{
	return _isNull;
}

FixedString64 NodeValue::toString() const
{
	FixedString64 str;
	if (_isNull)
	{
		str.append("null");
		return str;
	}
	switch (Type)
	{
	case ValueType::Bool:
		str.appendFormat("%d", _boolValue);
		break;
	case ValueType::Int:
		str.appendFormat("%d", _intValue);
		break;
	case ValueType::String:
		str.append(stringValue.c_str());
		break;
	case ValueType::Float:
		str.appendFormat("%f", _floatValue);
		break;
	case ValueType::Color:
		str.appendFormat("[%d,%d,%d,%d]", _colorValue.R, _colorValue.G, _colorValue.B, _colorValue.A);
		break;
	default:
		str.append("[unknown]");
		break;
	}
	return str;
}

NodeValue::~NodeValue()
{
}
