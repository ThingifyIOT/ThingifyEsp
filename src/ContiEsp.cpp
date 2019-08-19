#include "ContiEsp.h"
#include <Arduino.h>
#include <functional>



using namespace std::placeholders;

AsyncClient wifiTcpClient;


ContiEsp::ContiEsp(const char* deviceId, const char *deviceName):
Conti(deviceId, deviceName, wifiTcpClient)
{
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(false);
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);

	_wifi_multi.OnStateChanged = std::bind(&ContiEsp::OnWifiStateChanged, this, _1, _2);
}


void ContiEsp::Start()
{
	Conti::Start();
}

void ContiEsp::Loop()
{
	_wifi_multi.run();

	const wl_status_t wl_status = WiFi.status();
	if (_previousWlanStatus != wl_status)
	{
		_logger.info(LogComponent::Wifi, L("Status changed %s -> %s"),
			ContiUtils::WlStatusToStr(_previousWlanStatus),
			ContiUtils::WlStatusToStr(wl_status));
		_previousWlanStatus = wl_status;
	}

	Conti::Loop();
}

void ContiEsp::OnWifiStateChanged(WifiMultiState state, FixedStringBase& networkName)
{
	if (state == WifiMultiState::Connecting)
	{
		_logger.info(L("Wifi state changed to Connecting"));
		Conti::OnNetworkConnecting(networkName);
	}
	if (state == WifiMultiState::Searching)
	{
		_logger.info(L("Wifi state changed to Searching"));
		Conti::OnNetworkDisconnected();
	}
	if (state == WifiMultiState::Connected)
	{
		_logger.info(L("Wifi state changed to Connected"));
		Conti::OnNetworkConnected();
	}
}
bool ContiEsp::IsNetworkConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

void ContiEsp::AddAp(const char* ssid, const char* password)
{
	_wifi_multi.addAP(ssid, password);
}

void ContiEsp::AddApList(char* accessPoints[][2])
{
	for (int i = 0; accessPoints[i] != nullptr; i++)
	{
		const auto ssid = accessPoints[i][0];
		const auto password = accessPoints[i][1];
		if (ssid == nullptr)
		{
			return;
		}
		_wifi_multi.addAP(ssid, password);
	}
}

uint64_t ContiEsp::WatchdogTimeoutInMs()
{
	return 3000;
}
