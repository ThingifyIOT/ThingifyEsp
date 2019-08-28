
#include "Value/NodeValue.h"
#include <string.h>
#include "NodeValue.h"



NodeValue::NodeValue(ContiType type, bool isNull):_isNull(isNull)
{
	Type = type;
	switch(Type)
	{
	case ContiType::Bool:
		_boolValue = false;
		break;
	case ContiType::Int:
		_intValue = 0;
		break;
	case ContiType::String:
		stringValue = "";
		break;
	case ContiType::Float:
		_floatValue = 0.0f;
		break;
	case ContiType::Color:
		_colorValue.R = 0;
		_colorValue.G = 0;
		_colorValue.B = 0;
		_colorValue.A = 0;
		break;
	case ContiType::TimeSpan:
		_timespanValue = 0;
		break;

	case ContiType::Location: break;
	case ContiType::DateTime: break;
	default: ;
	}
}

NodeValue::NodeValue(const NodeValue& other)
{
	Type = other.Type;
	_isNull = other._isNull;

	switch (Type)
	{
	case ContiType::Bool:
		_boolValue = other._boolValue;
		return;
	case ContiType::Int:
		_intValue = other._intValue;
		return;
	case ContiType::String:
		stringValue = other.stringValue;
		return;
	case ContiType::Float:
		_floatValue = other._floatValue;
		return;
	case ContiType::Color:
		_colorValue = other._colorValue;
		return;
	case ContiType::TimeSpan:
		_timespanValue = other._timespanValue;
		return;
	case ContiType::Location:
		break;
	case ContiType::DateTime:
		break;
	}
}

NodeValue & NodeValue::operator=(NodeValue & other)
{
	Type = other.Type;
	_isNull = other._isNull;
	switch (Type)
	{
	case ContiType::Bool:
		_boolValue = other._boolValue;
		break;
	case ContiType::Int:
		_intValue = other._intValue;
		break;
	case ContiType::String:
		stringValue = other.stringValue;
		break;
	case ContiType::Float:
		_floatValue = other._floatValue;
		break;
	case ContiType::Color:
		_colorValue = other._colorValue;
		break;
	case ContiType::TimeSpan:
		_timespanValue = other._timespanValue;
		break;
	case ContiType::Location:
		break;
	case ContiType::DateTime:
		break;
	}
	return *this;
}

NodeValue NodeValue::Boolean(bool boolValue)
{
	NodeValue value(ContiType::Bool);
	value._boolValue = boolValue;
	return value;
}

NodeValue NodeValue::String(const char* stringValue)
{
	NodeValue value(ContiType::String);
	value.stringValue = FixedString<ContiConstants::MaxStringValueSize>(stringValue);
	return value;
}

NodeValue NodeValue::Int(int integerValue)
{
	NodeValue value(ContiType::Int);
	value._intValue = integerValue;
	return value;
}

NodeValue NodeValue::Range(int integerValue)
{
	NodeValue value(ContiType::Int);
	value._intValue = integerValue;
	return value;
}

NodeValue NodeValue::Float(float floatValue)
{
	NodeValue value(ContiType::Float);
	value._floatValue = floatValue;
	return value;
}
NodeValue NodeValue::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	NodeValue value(ContiType::Color);
	value._colorValue.R = r;
	value._colorValue.G = g;
	value._colorValue.B = b;
	value._colorValue.A = a;
	return value;
}
NodeValue NodeValue::TimeSpan(uint64_t ms)
{
	NodeValue value(ContiType::TimeSpan);
	value._timespanValue = ms * 10000;
	return value;
}
NodeValue NodeValue::NullString()
{
	NodeValue value(ContiType::String, true);
	return value;
}
NodeValue NodeValue::NullInt()
{
	NodeValue value(ContiType::Int, true);
	return value;
}
NodeValue NodeValue::NullFloat()
{
	NodeValue value(ContiType::Float, true);
	return value;
}
NodeValue NodeValue::NullColor()
{
	NodeValue value(ContiType::Color, true);
	return value;
}
NodeValue NodeValue::NullTimeSpan()
{
	NodeValue value(ContiType::TimeSpan, true);
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
	case ContiType::Bool:
		return _boolValue == other._boolValue;
	case ContiType::Int:
		return _intValue == other._intValue;
	case ContiType::String:		
		return strcmp(stringValue.c_str(), other.stringValue.c_str()) == 0;
	case ContiType::Float:
		return _floatValue == other._floatValue;	
	default:
		return false;
	}
}

bool NodeValue::IsNull()
{
	return _isNull;
}

FixedString<50> NodeValue::toString() const
{
	FixedString<50> str;
	if (_isNull)
	{
		str.append("null");
		return str;
	}
	switch (Type)
	{
	case ContiType::Bool:
		str.appendFormat("%d", _boolValue);
		break;
	case ContiType::Int:
		str.appendFormat("%d", _intValue);
		break;
	case ContiType::String:
		str.append(stringValue.c_str());
		break;
	case ContiType::Float:
		str.appendFormat("%f", _floatValue);
		break;
	case ContiType::Color:
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
