#ifndef _LOGGER_H
#define _LOGGER_H

#include "LogComponent.h"
#include "LogLevel.h"
#include <cstdarg>
#include <WString.h>
#include <pgmspace.h>

#define L(string_literal) (reinterpret_cast<const __FlashStringHelper*>(F(string_literal)))


class Logger
{
	LogComponent _currentComponent;
	LogLevel _currrentLevel;
	bool ShouldIncludeEntry(LogComponent component, LogLevel level);
	void log(LogComponent component, LogLevel level, const __FlashStringHelper* format, bool skipNewLine, va_list argptr);
public:
	Logger(Logger const &) = delete;
	void operator=(Logger const &x) = delete;
	LogComponent ComponentsToInclude;
	bool LogEnabled = true;
	Logger();
	LogLevel Level;
	void ForceIncludeComponent(LogComponent logComponent);
	void debug(const __FlashStringHelper * format, ...);
	void debug(LogComponent component, const __FlashStringHelper* format, ...);

	void info(const __FlashStringHelper* format, ...);
	void info(LogComponent component, const __FlashStringHelper* format, ...);
	void infoBegin(const __FlashStringHelper* format, ...);
	void infoBegin(LogComponent component, const __FlashStringHelper* format, ...);
	void add(const __FlashStringHelper* format, ...);
	void end();
	void warn(const __FlashStringHelper* format, ...);
	void err(const __FlashStringHelper* format, ...);
};

extern Logger LoggerInstance;

#endif
