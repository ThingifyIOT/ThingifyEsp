#include "ThingifyEspWifiMulti.h"
#include <limits.h>
#include <string.h>
#include "ThingifyUtils.h"

#include <esp_wifi.h>

ThingifyEspWiFiMulti::ThingifyEspWiFiMulti():
_logger(LoggerInstance)
{
	_stateChangeTime = millis();
	_state = WifiMultiState::NotConnected;
}

void ThingifyEspWiFiMulti::run()
{
	if (_state == WifiMultiState::Connecting)
	{	
		if (WiFi.status() == WL_CONNECTED)
		{
			FixedString32 ssid = WiFi.SSID().c_str();
			ChangeState(WifiMultiState::Connected, ssid);
			IPAddress ip = WiFi.localIP();
			uint8_t * mac = WiFi.BSSID();

			_logger.info(L("[WIFI] Sucessfully connected to WiFi, SSID: %s, IP: %d.%d.%d.%d, MAC: %02X:%02X:%02X:%02X:%02X:%02X"), 
				WiFi.SSID().c_str(), 
				ip[0], ip[1], ip[2], ip[3], 
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		else
		{
			if (millis() - _stateChangeTime > 15000)
			{				
				_logger.err(L("[WIFI] Connecting to network timed out, changing state to NotConnected"));
				PrintWifiError();
				ChangeState(WifiMultiState::NotConnected, _emptySsid);
				return;
			}	
		}
		return;
	}

	if (_state == WifiMultiState::Connected)
	{
		if (WiFi.status() != WL_CONNECTED)
		{
			ChangeState(WifiMultiState::NotConnected, _emptySsid);
		}
		return;
	}

	if(_state == WifiMultiState::NotConnected)
	{	
		_logger.info(L("[WIFI] Initiating network scan..."));

		// without this - first network scan times out
		WiFi.disconnect(false, false);
		// we set 5000 timeout per channel as not expecting to hit it - timeout handling is done in WifiMultiState::Scanning handler
		WiFi.scanNetworks(true, false, false, 5000);
		ChangeState(WifiMultiState::Scanning, _emptySsid);
	}


	if(_state == WifiMultiState::Scanning)
	{
		int16_t scanResult = WiFi.scanComplete();
		if(scanResult == WIFI_SCAN_RUNNING) 
		{
			if (millis() - _stateChangeTime > 15000)
			{
				_logger.err(L("[WIFI] Scan networks timed out"));
				WiFi.scanDelete();
				ChangeState(WifiMultiState::NotConnected, _emptySsid);
				return;
			}
			return;
		}
		if(scanResult < 0)
		{
			_logger.err(L("[WIFI] Scan networks failed: %d"), scanResult);
			WiFi.scanDelete();
			ChangeState(WifiMultiState::NotConnected, _emptySsid);
			return;
		}
		if(scanResult >= 0)
		{
			yield();
			WifiCredential bestNetwork("","");
			PrintResultsAndFindBestNetwork(scanResult, bestNetwork);
			WiFi.scanDelete();
			yield();

			if(bestNetwork.Name.length() == 0) 
			{
				_logger.debug(L("[WIFI] no matching wifi found!"));
				ChangeState(WifiMultiState::NotConnected, _emptySsid);
				return;
			}

			ChangeState(WifiMultiState::Connecting, bestNetwork.Name);
			_logger.info(L("[WIFI] Connecting to '%s' network..."), bestNetwork.Name.c_str());
			WiFi.begin(bestNetwork.Name.c_str(), bestNetwork.Password.c_str());
		}
	}
}

void ThingifyEspWiFiMulti::PrintResultsAndFindBestNetwork(int16_t scanResult, WifiCredential& bestNetwork)
{
	int bestNetworkDb = INT_MIN;
	uint8_t bestBSSID[6];
	int32_t bestChannel;

	if(scanResult == 0) 
	{
		_logger.info(L("[WIFI] No networks found"));
	}
	else
	{
		_logger.info(L("[WIFI] %d networks found"), scanResult);
	}

	if(_wifiCredentials == nullptr || _wifiCredentials->size() == 0)
	{
		_logger.warn(L("[WIFI] Credentials list not set or empty"));
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
		bool known = false;

		if(_wifiCredentials != nullptr)
		{
			for(uint32_t i = 0; i < _wifiCredentials->size(); i++) 
			{
				WifiCredential* entry = _wifiCredentials->at(i);
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

							bestNetwork.Name = entry->Name;
							bestNetwork.Password = entry->Password;
							memcpy((void*) &bestBSSID, (void*) BSSID_scan, sizeof(bestBSSID));
						}
					}
					break;
				}
			}
		}

		if(known) 
		{
			_logger.info(L(" ---> "));
		}

#ifdef ESP8266
		bool isEncrypted = sec_scan == ENC_TYPE_NONE;
#else
		bool isEncrypted = sec_scan == WIFI_AUTH_OPEN;
#endif


		_logger.info(L(" %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c"), 
			i, chan_scan, 
			BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5], 
			ssid_scan.c_str(), rssi_scan, (isEncrypted) ? ' ' : '*');
		delay(0);
	}
}

// ##################################################################################
void ThingifyEspWiFiMulti::SetWifiCredentialList(std::vector<WifiCredential*>* credentialList)
{
	_wifiCredentials = credentialList;
}
 
void ThingifyEspWiFiMulti::PrintWifiError()
{
	auto status = WiFi.status();

	switch (status)
	{
	case WL_NO_SSID_AVAIL:
		_logger.info(L("[WIFI] Connecting Failed AP not found"));
		break;
	case WL_CONNECT_FAILED:
		_logger.info(L("[WIFI] Connecting Failed."));
		break;
	default:
		_logger.info(L("[WIFI] Connecting Failed - status = %d"), status);
		break;
	}
}

void ThingifyEspWiFiMulti::ChangeState(WifiMultiState state, FixedStringBase& ssid)
{
	if (_state == state)
	{
		return;
	}
	_state = state;
	_stateChangeTime = millis();
	OnStateChanged(state, ssid);
}

