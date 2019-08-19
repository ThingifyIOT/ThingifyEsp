#ifndef _NODE_VALUE_H
#define _NODE_VALUE_H

#include "Node/NodeType.h"
#include <FixedString.h>
#include "ContiConstants.h"


class ContiColor
{
public:
	ContiColor()
	{
		A = R = G = B = 0;
	}

	ContiColor(const ContiColor& other)
	{
		R = other.R;
		G = other.G;
		B = other.B;
		A = other.A;
	}
	ContiColor& operator=(const ContiColor& other)
	{
		R = other.R;
		G = other.G;
		B = other.B;
		A = other.A;
	}
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
};
class NodeValue
{
private:
	bool _isNull;
public:
	union
	{
		bool _boolValue;
		int _intValue;
		double _floatValue;
		ContiColor _colorValue;
		uint64_t _timespanValue;
	};
	FixedString<ContiConstants::MaxStringValueSize> stringValue;

	ContiType Type;
	NodeValue(ContiType type, bool isNull = false);
	NodeValue(const NodeValue& other);
	NodeValue& operator=(NodeValue &other);
	~NodeValue();
	
	static NodeValue Boolean(bool boolValue);
	static NodeValue String(const char* stringValue);
	static NodeValue Int(int integerValue);
	static NodeValue Range(int integerValue);
	static NodeValue Float(float floatValue);
	static NodeValue Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
	static NodeValue TimeSpan(uint64_t ms);

	static NodeValue NullString();
	static NodeValue NullInt();
	static NodeValue NullFloat();
	static NodeValue NullColor();
	static NodeValue NullTimeSpan();
	
	bool IsSameAs(NodeValue& other) const;

	bool IsNull();
	bool AsBool() const;
	const char* AsString() const;
	int AsInt() const;
	float AsFloat() const;
	int RangeValue() const;
	ContiColor AsColor() const;

	FixedString<50> toString() const;
};

#endif

