#ifndef _NODE_TYPE_H
#define _NODE_TYPE_H

enum class NodeType : unsigned char
{
	BasicValue = 0,
	Range = 1,
	Enum = 2,
	Function = 3
};


enum class ValueType : unsigned char
{
	Bool = 0,
	Int = 1,
	String = 2,
	Float = 3,
	Location = 4,
	DateTime = 5,
	TimeSpan = 6,
	Color = 7
};

#endif
