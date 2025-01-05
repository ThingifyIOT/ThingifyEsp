#ifndef _THINGIFY_UTILS_H
#define _THINGIFY_UTILS_H

#include <cinttypes>
#include "ThingState.h"
#include "ThingError.h"
#include <FixedString.h>
#include <ThingifyConstants.h>
#include "Settings/ThingSettings.h"
#include "Logging/Logger.h"
#include <Wire.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFiType.h>
#endif

enum class NodeType : unsigned char;

class ThingifyUtils
{
public:
	static void PrintHex(const void* data, int count);
	static const __FlashStringHelper* ThingStateToStr(ThingState thingState);
	static const __FlashStringHelper* ThingStateToShortStr(ThingState thingState);
	static const __FlashStringHelper* WlStatusToStr(wl_status_t wlStatus);
	static void ScanI2C(TwoWire& wire);

	static FixedString32 TimeToShortStr(ulong timeInSeconds);
	static const __FlashStringHelper * NodeTypeToStr(NodeType nodeType);
	static void LogSettings(Logger& logger, ThingSettings& settings);
	static const __FlashStringHelper* ThingErrorToStr(ThingError error);

	static void RestartDevice();
};



class Stopwatch
{
	uint64_t _startMs;
	const char* _name;
	
public:
	Stopwatch(const char* name);
	~Stopwatch();
};


class SoftTimer
{
	uint64_t _ticks;
	int _delay;
	bool _isElapsed;
	void Tick();
public:
	SoftTimer(int delay);
	void UpdateDelay(int delay)
	{
		_delay = delay;
	}
	void SetElapsed();
	bool IsElapsed();
};

class ElapsedTimer
{
	uint64_t _time;
	bool _isStarted;
public:

	ElapsedTimer():_time(0)
	{
		_isStarted = false;
	}
	bool IsStarted()
	{
		return _isStarted;
	}
	void Start()
	{
		_isStarted = true;
		_time = millis();
	}
	void Stop()
	{
		_isStarted = false;
	}
	uint64_t ElapsedMs()
	{
		uint64_t now = millis();
		return (now >= _time ? now - _time : 0);
	}
	uint64_t ElapsedSeconds()
	{
		return ElapsedMs() / 1000ull;
	}

};
#endif