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

const char* ThingifyUtils::ThingStateToStr(ThingState thingState)
{
	switch (thingState)
	{
	case ThingState::NotConfigured: return "NotConfigured";
	case ThingState::Configuring: return "Configuring";
	case ThingState::Disabled: return "Disabled";
	case ThingState::SearchingForNetwork: return "SearchingForNetwork";
	case ThingState::ConnectingToNetwork: return "ConnectingToNetwork";
	case ThingState::ConnectingToMqtt: return "ConnectingToMqtt";
	case ThingState::Online: return "Online";
	case ThingState::DisconnectedFromMqtt: return "DisconnectedFromMqtt";
	case ThingState::Authenticating: return "Authenticating";
	case ThingState::ServiceUnavailable: return "ServiceUnavailable";
	case ThingState::Error: return "Error";

	default:
		return "Unknown";
	}
}

const char* ThingifyUtils::ThingStateToShortStr(ThingState thingState)
{
	switch (thingState)
	{
	case ThingState::NotConfigured: return "NOT_CONF";
	case ThingState::Configuring: return "CONF";
	case ThingState::Disabled: return "DISABLED";
	case ThingState::SearchingForNetwork: return "NET_SEARCH";
	case ThingState::ConnectingToNetwork: return "WIFI_CONN";
	case ThingState::ConnectingToMqtt: return "MQTT_CONN";
	case ThingState::Online: return "ONLINE";
	case ThingState::DisconnectedFromMqtt: return "DISC_MQTT";
	case ThingState::Authenticating: return "AUTH";
	case ThingState::ServiceUnavailable: return "SVC_UNAV";
	case ThingState::Error: return "ERR";
	default:
		return "UNKNOWN";
	}
}

const char* ThingifyUtils::WlStatusToStr(wl_status_t wlStatus)
{
	switch (wlStatus)
	{
	case WL_NO_SHIELD: return "WL_NO_SHIELD";
	case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
	case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
	case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
	case WL_CONNECTED: return "WL_CONNECTED";
	case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
	case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
	case WL_DISCONNECTED: return "WL_DISCONNECTED";
	default: return "Unknown";
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

const char* ThingifyUtils::NodeTypeToStr(NodeType nodeType)
{
	switch (nodeType)
	{
		case NodeType::BasicValue: return "BasicValue";
		case NodeType::Range: return "Range";
		case NodeType::Enum: return "Enum";
		case NodeType::Function: return "Function";
		default: return  "[unknown]";
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


const char* ThingifyUtils::ThingErrorToStr(ThingError error)
{
	switch (error)
	{
	case ThingError::InvalidToken:		return "Invalid token";
	case ThingError::ProtocolViolation: return "Protocol violation";
	case ThingError::OtherLoginError:	return "Other login error";
	case ThingError::StringOverflow:	return "String overflow";
	case ThingError::ListOverflow:		return "List overflow";
	case ThingError::LowFreeHeap:		return "Low free heap";
	case ThingError::LowFreeStack:		return "Low free stack";
	default: return "No error str";
	}

}

#ifdef ESP8266
const char* ThingifyUtils::WlDisconnectReasonToString(WiFiDisconnectReason reason)
{
	switch (reason)
	{
	case WIFI_DISCONNECT_REASON_UNSPECIFIED: return "WIFI_DISCONNECT_REASON_UNSPECIFIED";
	case WIFI_DISCONNECT_REASON_AUTH_EXPIRE: return "WIFI_DISCONNECT_REASON_AUTH_EXPIRE";
	case WIFI_DISCONNECT_REASON_AUTH_LEAVE: return "WIFI_DISCONNECT_REASON_AUTH_LEAVE";
	case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE: return "WIFI_DISCONNECT_REASON_ASSOC_EXPIRE";
	case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY: return "WIFI_DISCONNECT_REASON_ASSOC_TOOMANY";
	case WIFI_DISCONNECT_REASON_NOT_AUTHED: return "WIFI_DISCONNECT_REASON_NOT_AUTHED";
	case WIFI_DISCONNECT_REASON_NOT_ASSOCED: return "WIFI_DISCONNECT_REASON_NOT_ASSOCED";
	case WIFI_DISCONNECT_REASON_ASSOC_LEAVE: return "WIFI_DISCONNECT_REASON_ASSOC_LEAVE";
	case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED: return "WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED";
	case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD: return "WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD";
	case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD: return "WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD";
	case WIFI_DISCONNECT_REASON_IE_INVALID: return "WIFI_DISCONNECT_REASON_IE_INVALID";
	case WIFI_DISCONNECT_REASON_MIC_FAILURE: return "WIFI_DISCONNECT_REASON_MIC_FAILURE";
	case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT: return "WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT";
	case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT: return "WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT";
	case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS: return "WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS";
	case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID: return "WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID";
	case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID: return "WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID";
	case WIFI_DISCONNECT_REASON_AKMP_INVALID: return "WIFI_DISCONNECT_REASON_AKMP_INVALID";
	case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION: return "WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION";
	case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP: return "WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP";
	case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED: return "WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED";
	case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED: return "WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED";
	case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT: return "WIFI_DISCONNECT_REASON_BEACON_TIMEOUT";
	case WIFI_DISCONNECT_REASON_NO_AP_FOUND: return "WIFI_DISCONNECT_REASON_NO_AP_FOUND";
	case WIFI_DISCONNECT_REASON_AUTH_FAIL: return "WIFI_DISCONNECT_REASON_AUTH_FAIL";
	case WIFI_DISCONNECT_REASON_ASSOC_FAIL: return "WIFI_DISCONNECT_REASON_ASSOC_FAIL";
	case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT: return "WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT";
	default: return "Unknown";
	}
}
#endif


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
