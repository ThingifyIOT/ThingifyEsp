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
	Connecting, Searching, Connected
};

class ThingifyEspWiFiMulti 
{
private:
	WifiMultiState _state;
	uint64_t _stateChangeTime = 0;

	std::vector<WifiCredential*>* _additionalWifiCredentials;
	std::vector<WifiCredential*> _wifiCredentials;

	void ChangeState(WifiMultiState state, FixedStringBase& ssid);
	void PrintConnectResult(wl_status_t status);

public:
	ThingifyEspWiFiMulti();

	bool AddWifiCredential(const char* ssid, const char *passphrase = nullptr);
	void SetAdditionalWifiCredentialList(std::vector<WifiCredential*>* credentialList);
    void run();
	std::function<void(WifiMultiState, FixedStringBase&)> OnStateChanged;
	Logger& _logger;
};

#endif
