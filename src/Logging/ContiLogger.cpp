#include "ContiLogger.h"
#include "LogUtils.h"

#include <Arduino.h>

ContiLogger::ContiLogger()
{
	Level = LogLevel::Info;
	ComponentsToInclude = static_cast<LogComponent>(
		static_cast<int>(LogComponent::Packet)
	);
}

bool ContiLogger::ShouldIncludeEntry(LogComponent component, LogLevel level)
{
	const auto includeRegardlessLevel = (static_cast<int>(ComponentsToInclude) & static_cast<int>(component)) > 0;
	if (level < Level && !includeRegardlessLevel)
	{
		return false;
	}
	return true;
}

void ContiLogger::log(LogComponent component, LogLevel level, const __FlashStringHelper* format, bool skipNewLine, va_list argptr)
{
	_currentComponent = component;
	_currrentLevel = level;

	if (!ShouldIncludeEntry(component, level))
	{
		return;
	}
	
	auto time = millis();
	Serial.printf("%02lu.%03lu: ", time / 1000, time % 1000);

	const auto levelStr = LogUtils::LogLevelToString(level);
	if(levelStr != nullptr)
	{
		Serial.printf("[%s] ", levelStr);
	}

	const auto componentStr = LogUtils::ComponentToString(component);
	if(componentStr != nullptr)
	{
		Serial.printf("[%s] ", componentStr);
	}
	char logBuffer[200];
	vsnprintf_P(logBuffer, 200, (PGM_P)format, argptr);
	if (skipNewLine)
	{
		Serial.print(logBuffer);
	}
	else
	{
		Serial.println(logBuffer);
	}
	Serial.flush();
}

void ContiLogger::ForceIncludeComponent(LogComponent logComponent)
{
	ComponentsToInclude = static_cast<LogComponent>(
		static_cast<int>(ComponentsToInclude) |static_cast<int>(logComponent));
}


void ContiLogger::info(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(LogComponent::Other, LogLevel::Info, format, false, argptr);
	va_end(argptr);
}


void ContiLogger::info(LogComponent component, const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(component, LogLevel::Info, format, false, argptr);
	va_end(argptr);
}

void ContiLogger::infoBegin(const __FlashStringHelper * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(LogComponent::Other, LogLevel::Info, format, true, argptr);
	va_end(argptr);
}

void ContiLogger::infoBegin(LogComponent component, const __FlashStringHelper * format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(component, LogLevel::Info, format, true, argptr);
	va_end(argptr);
}

void ContiLogger::add(const __FlashStringHelper * format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	if (ShouldIncludeEntry(_currentComponent, _currrentLevel))
	{
		char logBuffer[200];
		vsnprintf_P(logBuffer, 200, (PGM_P)format, argptr);
		Serial.print(logBuffer);
	}
	va_end(argptr);
}

void ContiLogger::end()
{
	if (!ShouldIncludeEntry(_currentComponent, _currrentLevel))
	{
		return;
	}
	Serial.println();
	Serial.flush();
}


void ContiLogger::warn(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(LogComponent::Other, LogLevel::Warning, format, false, argptr);
	va_end(argptr);
}

void ContiLogger::err(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(LogComponent::Other, LogLevel::Error, format, false, argptr);
	va_end(argptr);
}

void ContiLogger::debug(const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(LogComponent::Other, LogLevel::Debug, format, false, argptr);
	va_end(argptr);
}

void ContiLogger::debug(LogComponent component, const __FlashStringHelper* format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	log(component, LogLevel::Debug, format, false, argptr);
	va_end(argptr);
}

ContiLogger ContiLoggerInstance;

