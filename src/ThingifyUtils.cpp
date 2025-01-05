#include "ThingifyUtils.h"
#include <Arduino.h>
#include "Node/NodeType.h"
#include <EEPROM.h>

void ThingifyUtils::PrintHex(const void* data, int count)
{
	for (int i = 0; i < count; i++)
	{
		Serial.printf("%x ", reinterpret_cast<const char*>(data)[i]);
	}
	Serial.println();
}

const __FlashStringHelper* ThingifyUtils::ThingStateToStr(ThingState thingState)
{
	switch (thingState)
	{
	case ThingState::NotConfigured: return F("NotConfigured");
	case ThingState::Configuring: return F("Configuring");
	case ThingState::Disabled: return F("Disabled");
	case ThingState::SearchingForNetwork: return F("SearchingForNetwork");
	case ThingState::ConnectingToNetwork: return F("ConnectingToNetwork");
	case ThingState::ConnectingToMqtt: return F("ConnectingToMqtt");
	case ThingState::Online: return F("Online");
	case ThingState::DisconnectedFromMqtt: return F("DisconnectedFromMqtt");
	case ThingState::Authenticating: return F("Authenticating");
	case ThingState::ServiceUnavailable: return F("ServiceUnavailable");
	case ThingState::Error: return F("Error");

	default:
		return F("Unknown");
	}
}

const __FlashStringHelper* ThingifyUtils::ThingStateToShortStr(ThingState thingState)
{
	switch (thingState)
	{
	case ThingState::NotConfigured: return F("NOT_CONF");
	case ThingState::Configuring: return F("CONF");
	case ThingState::Disabled: return F("DISABLED");
	case ThingState::SearchingForNetwork: return F("NET_SEARCH");
	case ThingState::ConnectingToNetwork: return F("WIFI_CONN");
	case ThingState::ConnectingToMqtt: return F("MQTT_CONN");
	case ThingState::Online: return F("ONLINE");
	case ThingState::DisconnectedFromMqtt: return F("DISC_MQTT");
	case ThingState::Authenticating: return F("AUTH");
	case ThingState::ServiceUnavailable: return F("SVC_UNAV");
	case ThingState::Error: return F("ERR");
	default:
		return F("UNKNOWN");
	}
}

const __FlashStringHelper* ThingifyUtils::WlStatusToStr(wl_status_t wlStatus)
{
	switch (wlStatus)
	{
	case WL_NO_SHIELD: return F("WL_NO_SHIELD");
	case WL_IDLE_STATUS: return F("WL_IDLE_STATUS");
	case WL_NO_SSID_AVAIL: return F("WL_NO_SSID_AVAIL");
	case WL_SCAN_COMPLETED: return F("WL_SCAN_COMPLETED");
	case WL_CONNECTED: return F("WL_CONNECTED");
	case WL_CONNECT_FAILED: return F("WL_CONNECT_FAILED");
	case WL_CONNECTION_LOST: return F("WL_CONNECTION_LOST");
	case WL_DISCONNECTED: return F("WL_DISCONNECTED");
	default: return F("Unknown");
	}
}

void ThingifyUtils::ScanI2C(TwoWire & wire)
{
	int nDevices = 0;
	Serial.println("\nLooking for I2C devices:");
	for (byte address = 0; address < 127; address++)
	{
		wire.beginTransmission(address);
		if (wire.endTransmission() == 0)
		{
			Serial.print("I2c device found: 0x");
			Serial.print(address, HEX);
			Serial.println();
			nDevices++;
		}
	}

	Serial.printf("Found %d I2C devices\n", nDevices);

}

FixedString32 ThingifyUtils::TimeToShortStr(ulong timeInSeconds)
{
	FixedString32 buffer;

	const int secondsInHour = 60 * 60;

	int hours = timeInSeconds / secondsInHour;
	int minutes = (timeInSeconds % secondsInHour) / 60;
	int seconds = (timeInSeconds % 60);
	if (hours > 0)
	{
		buffer.appendFormat("%dh%dm%ds", hours, minutes, seconds);
	}
	else if (minutes > 0)
	{
		buffer.appendFormat("%dm%ds", minutes, seconds);
	}
	else
	{
		buffer.appendFormat("%ds", seconds);
	}
	return buffer;
}

const __FlashStringHelper* ThingifyUtils::NodeTypeToStr(NodeType nodeType)
{
	switch (nodeType)
	{
		case NodeType::BasicValue: return F("BasicValue");
		case NodeType::Range: return F("Range");
		case NodeType::Enum: return F("Enum");
		case NodeType::Function: return F("Function");
		default: return  F("[unknown]");
	}
}

void ThingifyUtils::LogSettings(Logger& logger, ThingSettings& settings)
{
	logger.info(F("Settings: api = %s:%d, token = %s, %d wifi networks"), 
		settings.ApiServer.c_str(),
		settings.ApiPort,
		settings.Token.c_str(), 
		settings.WifiNetworks.size());

	for(int i=0; i < settings.WifiNetworks.size(); i++)
	{
		logger.info(F(" Network%d: %s, %s"),
		 	i+1, 
			settings.WifiNetworks[i]->Name.c_str(), 
			settings.WifiNetworks[i]->Password.c_str());
	}
}


const __FlashStringHelper* ThingifyUtils::ThingErrorToStr(ThingError error)
{
	switch (error)
	{
	case ThingError::InvalidToken:		return F("Invalid token");
	case ThingError::ProtocolViolation: return F("Protocol violation");
	case ThingError::OtherLoginError:	return F("Other login error");
	case ThingError::StringOverflow:	return F("String overflow");
	case ThingError::ListOverflow:		return F("List overflow");
	case ThingError::LowFreeHeap:		return F("Low free heap");
	case ThingError::LowFreeStack:		return F("Low free stack");
	default: return F("No error str");
	}

}



void ThingifyUtils::RestartDevice()
{
#if ESP8266
	ESP.reset();
#else
	ESP.restart();
#endif
}


Stopwatch::Stopwatch(const char* name):
	_name(name)
{
	_startMs = millis();
}

Stopwatch::~Stopwatch()
{
	int elapsed = millis()-_startMs;
	if (elapsed < 2)
	{
		return;
	}
	Serial.printf("Time %s: %d ms\n", _name, elapsed);
}

SoftTimer::SoftTimer(int delay)
{
	_delay = delay;
	_ticks = 0;
	_isElapsed = false;
}

void SoftTimer::Tick()
{
	auto ms = millis();
	if (ms - _ticks < _delay)
	{
		return;
	}
	_isElapsed = true;
	_ticks = ms;

}

void SoftTimer::SetElapsed()
{
	_isElapsed = true;
}

bool SoftTimer::IsElapsed()
{
	Tick();
	auto isElapsed = _isElapsed;
	_isElapsed = false;
	return isElapsed;
}
