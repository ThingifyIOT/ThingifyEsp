#include "ThingifyEsp.h"
#include <Arduino.h>
#include <functional>
#include "Modules/DiagnosticsModule.h"

using namespace std::placeholders;

AsyncClient wifiTcpClient;


ThingifyEsp::ThingifyEsp(const char* deviceName):
_espZeroConfiguration(_settingsStorage,  Properties),
Thingify(deviceName, wifiTcpClient)
{
	StartNetwork();

	_wifiMulti.OnStateChanged = std::bind(&ThingifyEsp::OnWifiStateChanged, this, _1, _2);
}

void ThingifyEsp::SetNetworkState(NetworkState state)
{
	if(state == _networkState)
	{
		return;
	}
	auto prevState = _networkState;
	_networkState = state;
	switch (_networkState)
	{
		case NetworkState::Disabled:
			_logger.info(L("Network state changed to Disabled"));
			if(prevState != NetworkState::Disconnected)
			{
				Thingify::OnNetworkDisconnected();
			}
		case NetworkState::Disconnected:		
			_logger.info(L("Network state changed to Disconnected"));
			if(prevState != NetworkState::Disabled)
			{
				Thingify::OnNetworkDisconnected();
			}
			break;
		case NetworkState::Connected:
			_logger.info(L("Network state changed to Connected"));
			Thingify::OnNetworkConnected();
			break;
		case NetworkState::Connecting:
			_logger.info(L("Network state changed to Connecting"));
			Thingify::OnNetworkConnecting();
			break;
	}
}

void ThingifyEsp::Start()
{
	Thingify::Start();
}

void ThingifyEsp::Loop()
{
	auto currentState = GetCurrentState();
	if(currentState != ThingState::Disabled && 
	   currentState != ThingState::NotConfigured && 
	   currentState != ThingState::Configuring)
	{
		_wifiMulti.run();
	}

	const wl_status_t wl_status = WiFi.status();
	if (_previousWlanStatus != wl_status)
	{
		_logger.info(L("[WiFi] Status changed %s -> %s"),
			ThingifyUtils::WlStatusToStr(_previousWlanStatus),
			ThingifyUtils::WlStatusToStr(wl_status));
		_previousWlanStatus = wl_status;
	}

	Thingify::Loop();
	_espZeroConfiguration.Loop();
}

void ThingifyEsp::StartNetwork()
{
	_logger.info(F("StartNetwork"));
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(false);
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);

	SetNetworkState(NetworkState::Disconnected);	
}

void ThingifyEsp::StopNetwork()
{
	_logger.info(F("StopNetwork"));
	SetNetworkState(NetworkState::Disabled);
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(false);
	WiFi.persistent(false);
#ifdef ESP32
	WiFi.mode(WIFI_MODE_NULL);
#elif ESP8266
	WiFi.mode(WIFI_OFF);
#endif
}

FixedStringBase& ThingifyEsp::GetNetworkName()
{
	return _networkName;
}

void ThingifyEsp::OnConfigurationLoaded()
{
	_logger.info(L("OnConfigurationLoaded"));

	

	_wifiMulti.SetWifiCredentialList(&_settings.WifiNetworks);
}

void ThingifyEsp::StartZeroConfiguration()
{
	_espZeroConfiguration.Start();
}
bool ThingifyEsp::IsZeroConfigurationReady()
{
	return _espZeroConfiguration.IsReady();
}
void ThingifyEsp::OnWifiStateChanged(WifiMultiState state, FixedStringBase& networkName)
{
	if (state == WifiMultiState::Connecting)
	{
		_networkName = networkName;
		_logger.info(L("Wifi state changed to Connecting"));
		SetNetworkState(NetworkState::Connecting);

	}
	if (state == WifiMultiState::NotConnected)
	{
		_logger.info(L("Wifi state changed to Searching"));
		SetNetworkState(NetworkState::Disconnected);
	}
	if (state == WifiMultiState::Scanning)
	{
		_logger.info(L("Wifi state changed to Scanning"));
		SetNetworkState(NetworkState::Disconnected);
	}
	if (state == WifiMultiState::Connected)
	{
		_networkName = networkName;
		_logger.info(L("Wifi state changed to Connected"));
		SetNetworkState(NetworkState::Connected);
	}
}
bool ThingifyEsp::IsNetworkConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

uint64_t ThingifyEsp::WatchdogTimeoutInMs()
{
	return 3000;
}

void ThingifyEsp::AddDiagnostics(int updateInteval)
{
	auto diagnostics = new DiagnosticsModule(*this);
	diagnostics->UpdateIntervalInMs = updateInteval;
	AddModule(diagnostics);
}
