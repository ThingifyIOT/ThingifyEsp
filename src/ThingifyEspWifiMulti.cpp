#include "ThingifyEspWifiMulti.h"
#include <limits.h>
#include <string.h>
#include "ThingifyUtils.h"

ThingifyEspWiFiMulti::ThingifyEspWiFiMulti():
_logger(LoggerInstance)
{
	_stateChangeTime = millis();
	_state = WifiMultiState::Searching;
}


void ThingifyEspWiFiMulti::run()
{    
    wl_status_t status = WiFi.status();

	if(_state != WifiMultiState::Connected)
	{
		if(status == WL_CONNECTED)
		{
			FixedString32 ssid = WiFi.SSID().c_str();
			ChangeState(WifiMultiState::Connected, ssid);
			return;
		}
	}

	if (_state == WifiMultiState::Connecting)
	{	
		if (millis() - _stateChangeTime > 7000)
		{
			PrintConnectResult(status);
			_logger.err(L("Connecting to network timed out, changing state to Searching"));
			
			FixedString16 emptySsid("");
			ChangeState(WifiMultiState::Searching, emptySsid);

			return;
		}	
		if (status == WL_CONNECTED)
		{
			FixedString32 ssid = WiFi.SSID().c_str();
			ChangeState(WifiMultiState::Connected, ssid);
			PrintConnectResult(status);
		}	
		return;
	}

	if (_state == WifiMultiState::Connected)
	{
		if (status != WL_CONNECTED)
		{
			FixedString16 emptySsid("");
			ChangeState(WifiMultiState::Searching, emptySsid);
		}
		return;
	}

	int8_t scanResult = WiFi.scanComplete();
	if(scanResult == WIFI_SCAN_RUNNING) 
	{		// scan is already running
		return;
	}
		
	if(scanResult < 0) 
	{
		// start scan
		_logger.debug(LogComponent::Wifi, L("delete old wifi config..."));
		WiFi.disconnect();

		_logger.info(L("Initiating network scan..."));
		// scan wifi async mode
		WiFi.scanNetworks(true);
		return;
	}
	_logger.info(LogComponent::Wifi, L("scan done"));
	delay(0);

	// scan done analyze
	WifiCredential* bestNetwork = nullptr;
	int bestNetworkDb = INT_MIN;
	uint8_t bestBSSID[6];
	int32_t bestChannel;

	if(scanResult == 0) 
	{
		_logger.info(LogComponent::Wifi, L("no networks found"));
	}
	else
	{
		_logger.info(LogComponent::Wifi, L("%d networks found"), scanResult);
	}

	for(int8_t i = 0; i < scanResult; ++i) 
	{
		String ssid_scan;
		int32_t rssi_scan;
		uint8_t sec_scan;
		uint8_t* BSSID_scan;
		int32_t chan_scan;

#if ESP8266
		bool hidden_scan = false;
		WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);
#else
		WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan);
#endif

		std::vector<WifiCredential*> credentials;
		credentials.reserve(_wifiCredentials.size() + (_additionalWifiCredentials == nullptr ? 0: _additionalWifiCredentials->size()));
		for(int i=0; i < _wifiCredentials.size(); i++)
		{
			credentials.push_back(_wifiCredentials[i]);
		}
		if(_additionalWifiCredentials != nullptr)
		{
			for(int i=0; i < _additionalWifiCredentials->size() ; i++)
			{
				credentials.push_back((*_additionalWifiCredentials)[i]);
			}
		}
	
		bool known = false;
		for(uint32_t x = 0; x < credentials.size(); x++) 
		{
			WifiCredential* entry = credentials[x];

			if(entry->Name.equals(ssid_scan.c_str()))
			{ // SSID match
				known = true;
				if(rssi_scan > bestNetworkDb) 
				{ // best network


#ifdef ESP8266
					if(sec_scan == ENC_TYPE_NONE || entry->Password.length() > 0) 
#else
					if (sec_scan == WIFI_AUTH_OPEN || entry->Password.length() > 0)
#endif 
					{ // check for passphrase if not open wlan
						bestNetworkDb = rssi_scan;
						bestChannel = chan_scan;
						bestNetwork = entry;
						memcpy((void*) &bestBSSID, (void*) BSSID_scan, sizeof(bestBSSID));
					}
				}
				break;
			}
		}

		if(known) 
		{
			_logger.debug(LogComponent::Wifi, L(" ---> "));
		}

#ifdef ESP8266
		bool isEncrypted = sec_scan == ENC_TYPE_NONE;
#else
		bool isEncrypted = sec_scan == WIFI_AUTH_OPEN;
#endif


		_logger.debug(LogComponent::Wifi, L(" %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c"), 
			i, chan_scan, 
			BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5], 
			ssid_scan.c_str(), rssi_scan, (isEncrypted) ? ' ' : '*');
		delay(0);
	}

	// clean up ram
	WiFi.scanDelete();
	delay(0);

	
	if(bestNetwork == nullptr) 
	{
		_logger.debug(LogComponent::Wifi, L("no matching wifi found!"));
		return;
	}

	//_logger.debug(LogComponent::Wifi, L("Connecting BSSID: %02X:%02X:%02X:%02X:%02X:%02X SSID: %s Channal: %d (%d)"), bestBSSID[0], bestBSSID[1], bestBSSID[2], bestBSSID[3], bestBSSID[4], bestBSSID[5], bestNetwork.ssid, bestChannel, bestNetworkDb);
	ChangeState(WifiMultiState::Connecting, bestNetwork->Name);

	_logger.info(L("Connecting to '%s' network..."), bestNetwork->Name.c_str());

	status = WiFi.begin(bestNetwork->Name.c_str(), bestNetwork->Password.c_str(), bestChannel, bestBSSID);
	return;
}

// ##################################################################################
void ThingifyEspWiFiMulti::SetAdditionalWifiCredentialList(std::vector<WifiCredential*>* credentialList)
{
	_additionalWifiCredentials = credentialList;

}

void ThingifyEspWiFiMulti::PrintConnectResult(wl_status_t status)
{
	IPAddress ip = WiFi.localIP();
	uint8_t * mac = WiFi.BSSID();

	switch (status)
	{
	case WL_CONNECTED:
		_logger.info(LogComponent::Wifi, L("Connecting done, SSID: %s, IP: %d.%d.%d.%d, MAC: %02X:%02X:%02X:%02X:%02X:%02X"), 
			WiFi.SSID().c_str(), 
			ip[0], ip[1], ip[2], ip[3], 
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		break;
	case WL_NO_SSID_AVAIL:
		_logger.info(LogComponent::Wifi, L("Connecting Failed AP not found"));
		break;
	case WL_CONNECT_FAILED:
		_logger.info(LogComponent::Wifi, L("Connecting Failed."));
		break;
	default:
		_logger.info(LogComponent::Wifi, L("Connecting Failed - status = %d"), status);
		break;
	}
}


bool ThingifyEspWiFiMulti::AddWifiCredential(const char* ssid, const char *passphrase) 
{
	_wifiCredentials.push_back(new WifiCredential(ssid, passphrase));    
    return true;
}


void ThingifyEspWiFiMulti::ChangeState(WifiMultiState state, FixedStringBase& ssid)
{
	if (_state == state)
	{
		return;
	}

	_logger.info(L("Wifi multi state change"));

	_state = state;
	_stateChangeTime = millis();
	OnStateChanged(state, ssid);
}

