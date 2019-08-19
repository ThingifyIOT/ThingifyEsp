#ifndef  _THING_ERROR_H
#define _THING_ERROR_H

enum class ThingError : uint8_t
{
	NoError,
	InvalidToken,
	ProtocolViolation,
	OtherLoginError,
	StringOverflow,
	ListOverflow,
	LowFreeHeap,
	LowFreeStack,
	Other
};

#endif

