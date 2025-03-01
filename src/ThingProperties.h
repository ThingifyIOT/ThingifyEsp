#ifndef _THING_PROPERTIES_H
#define _THING_PROPERTIES_H

#include <cstdint>

struct ThingProperties
{
	const char* DefaultName;
	uint64_t ConnectTime = 0;
	uint64_t ResetTime = 0;
};

#endif
