#include "ThingifyEsp.h"
#include <Arduino.h>
#include <functional>



using namespace std::placeholders;

AsyncClient wifiTcpClient;


ThingifyEsp::ThingifyEsp(const char *deviceName):
Thingify(deviceName, wifiTcpClient),
_espZeroConfiguration(Thingify::_settingsStorage)
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
		_logger.info(LogComponent::Wifi, L("Status changed %s -> %s"),
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
	_wifiMulti.SetAdditionalWifiCredentialList(&(_settings.WifiNetworks));
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
	if (state == WifiMultiState::Searching)
	{
		_logger.info(L("Wifi state changed to Searching"));
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

void ThingifyEsp::AddAp(const char* ssid, const char* password)
{
	_wifiMulti.AddWifiCredential(ssid, password);
}

void ThingifyEsp::AddApList(char* accessPoints[][2])
{
	for (int i = 0; accessPoints[i] != nullptr; i++)
	{
		const auto ssid = accessPoints[i][0];
		const auto password = accessPoints[i][1];
		if (ssid == nullptr)
		{
			return;
		}
		_wifiMulti.AddWifiCredential(ssid, password);
	}
}

uint64_t ThingifyEsp::WatchdogTimeoutInMs()
{
	return 3000;
}
