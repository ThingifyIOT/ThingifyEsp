#ifndef _CONTI_ESP_H
#define _CONTI_ESP_H

#include "Conti.h"
#include "ContiESP8266WiFiMulti.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

class ContiEsp : public Conti
{
	ContiWiFiMulti _wifi_multi;
	volatile bool _wlConnectOccured;
	volatile bool _wlDisconnectOccured;
	wl_status_t _previousWlanStatus = WL_NO_SHIELD;
	bool IsNetworkConnected();
public:
	ContiEsp(const char* deviceId, const char* deviceName);

	void Start() override;
	void Loop() override;

	void OnWifiStateChanged(WifiMultiState state, FixedStringBase& networkName);
	void AddAp(const char* ssid, const char* password = nullptr);
	void AddApList(char* accessPoints[][2]);	
	uint64_t WatchdogTimeoutInMs();
};

#endif
