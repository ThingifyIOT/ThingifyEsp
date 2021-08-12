#ifndef _THINGIFY_ESP_H
#define _THINGIFY_ESP_H

#include "Thingify.h"
#include "ThingifyEspWifiMulti.h"
#include "ZeroConfiguration/EspZeroConfiguration.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

enum class NetworkState
{
	Disabled,
	Disconnected,
	Connected,
	Connecting
};
class ThingifyEsp : public Thingify
{
private:
	NetworkState _networkState = NetworkState::Disabled;
	ThingifyEspWiFiMulti _wifiMulti;
	EspZeroConfiguration _espZeroConfiguration;
	volatile bool _wlConnectOccured;
	volatile bool _wlDisconnectOccured;
	wl_status_t _previousWlanStatus = WL_NO_SHIELD;
	bool IsNetworkConnected();	
	void StopNetwork() override;
	void StartNetwork() override;
    FixedStringBase& GetNetworkName() override;
	void OnConfigurationLoaded() override;
	void SetNetworkState(NetworkState state);
	FixedString32 _networkName;
public:
	ThingifyEsp(const char* deviceId, const char* deviceName);
    void AddDiagnostics(int updateInteval = 10000);

	void Start() override;
	void Loop() override;
	void StartZeroConfiguration() override;
	bool IsZeroConfigurationReady() override;
	void OnWifiStateChanged(WifiMultiState state, FixedStringBase& networkName);
	void AddApList(char* accessPoints[][2]);	
	uint64_t WatchdogTimeoutInMs();
	void AddAp(const char* ssid, const char* password);

};

#endif
