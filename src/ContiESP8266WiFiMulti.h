#ifndef _CONTI_ESP_WIFI_MULTI_H
#define _CONTI_ESP_WIFI_MULTI_H

#include <Arduino.h>
#include <Logging/Logger.h>
#include <FixedString.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#undef min
#undef max
#include <vector>

typedef struct 
{
        char * ssid;
        char * passphrase;
} WifiAPlist_t;

enum class WifiMultiState
{
	Connecting, Searching, Connected
};

class ContiWiFiMulti 
{
private:
	WifiMultiState _state;
	std::vector<WifiAPlist_t> APlist;
	bool APlistAdd(const char* ssid, const char *passphrase = nullptr);
	void APlistClean(void);
	void ChangeState(WifiMultiState state, FixedStringBase& ssid);
	uint64_t _stateChangeTime;
public:
	ContiWiFiMulti();
    bool addAP(const char* ssid, const char *passphrase = nullptr);
    void run();
	void PrintConnectResult(wl_status_t status);
	std::function<void(WifiMultiState, FixedStringBase&)> OnStateChanged;
	Logger& _logger;
};

#endif
