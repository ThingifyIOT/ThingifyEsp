#ifndef _THINGIFY_UTILS_H
#define _THINGIFY_UTILS_H

#include <cinttypes>
#include "ThingState.h"
#include "ThingError.h"
#include <FixedString.h>
#include <ThingifyConstants.h>

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
	static const char* ThingStateToStr(ThingState thingState);
	static const char* ThingStateToShortStr(ThingState thingState);
	static const char* WlStatusToStr(wl_status_t wlStatus);
	static void ScanI2C(TwoWire& wire);

	static FixedString32 TimeToShortStr(ulong timeInSeconds);
	static const char * NodeTypeToStr(NodeType nodeType);
#ifdef ESP8266
	static const char* WlDisconnectReasonToString(WiFiDisconnectReason reason);
#endif
	static const char* ThingErrorToStr(ThingError error);

	static void WriteRestartReason(FixedStringBase& errorStr);
	static void ReadRestartReason(FixedStringBase& errorStr);
	static void ClearRestartReason();
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