#ifndef _LOG_UTILS_H
#define _LOG_UTILS_H

#include "LogLevel.h"
#include <Logging/LogComponent.h>

class LogUtils
{
public:
	static const char* LogLevelToString(LogLevel level);
	static const char* ComponentToString(LogComponent component);
};

#endif 

