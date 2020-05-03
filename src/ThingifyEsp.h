#ifndef _THINGIFY_ESP_H
#define _THINGIFY_ESP_H

#include "Thingify.h"
#include "ThingifyEspWifiMulti.h"
#include "SmartConfigServer.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

class ThingifyEsp : public Thingify
{
private:
	ThingifyEspWiFiMulti _wifi_multi;
	SmartConfigServer _smartConfigServer;
	volatile bool _wlConnectOccured;
	volatile bool _wlDisconnectOccured;
	wl_status_t _previousWlanStatus = WL_NO_SHIELD;
	bool IsNetworkConnected();	
	void StopNetwork() override;
	void StartNetwork() override;
public:
	ThingifyEsp(const char* deviceName);

	void Start() override;
	void Loop() override;
	void OnWifiStateChanged(WifiMultiState state, FixedStringBase& networkName);
	void AddAp(const char* ssid, const char* password = nullptr);
	void AddApList(char* accessPoints[][2]);	
	uint64_t WatchdogTimeoutInMs();
};

#endif
