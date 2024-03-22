#ifndef _THINGIFY_ESP_WIFI_MULTI_H
#define _THINGIFY_ESP_WIFI_MULTI_H

#include <Arduino.h>
#include <Logging/Logger.h>
#include <FixedString.h>
#include <Settings/WifiCredential.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#undef min
#undef max
#include <vector>

enum class WifiMultiState
{
	Connecting, NotConnected, Connected, Scanning
};

class ThingifyEspWiFiMulti 
{
private:
	WifiMultiState _state;
	uint64_t _stateChangeTime = 0;
	FixedString16 _emptySsid;

	std::vector<WifiCredential*>* _wifiCredentials = nullptr;

	void ChangeState(WifiMultiState state, FixedStringBase& ssid);
	void PrintWifiError();
	void PrintResultsAndFindBestNetwork(int16_t scanResult, WifiCredential& bestNetwork);

public:
	ThingifyEspWiFiMulti();
	void SetWifiCredentialList(std::vector<WifiCredential*>* credentialList);
    void run();
	std::function<void(WifiMultiState, FixedStringBase&)> OnStateChanged;
	Logger& _logger;
};

#endif
