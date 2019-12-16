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

bool ThingifyEspWiFiMulti::addAP(const char* ssid, const char *passphrase)
{
    return APlistAdd(ssid, passphrase);
}

void ThingifyEspWiFiMulti::run()
{    
    wl_status_t status = WiFi.status();

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
	WifiAPlist_t bestNetwork{ nullptr, nullptr };
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

		bool known = false;
		for(uint32_t x = 0; x < APlist.size(); x++) 
		{
			WifiAPlist_t entry = APlist[x];

			if(ssid_scan == entry.ssid) 
			{ // SSID match
				known = true;
				if(rssi_scan > bestNetworkDb) 
				{ // best network


#ifdef ESP8266
					if(sec_scan == ENC_TYPE_NONE || entry.passphrase) 
#else
					if (sec_scan == WIFI_AUTH_OPEN || entry.passphrase)
#endif 
					{ // check for passphrase if not open wlan
						bestNetworkDb = rssi_scan;
						bestChannel = chan_scan;
						memcpy((void*) &bestNetwork, (void*) &entry, sizeof(bestNetwork));
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

	
	if(!bestNetwork.ssid) 
	{
		_logger.debug(LogComponent::Wifi, L("no matching wifi found!"));
		return;
	}

	//_logger.debug(LogComponent::Wifi, L("Connecting BSSID: %02X:%02X:%02X:%02X:%02X:%02X SSID: %s Channal: %d (%d)"), bestBSSID[0], bestBSSID[1], bestBSSID[2], bestBSSID[3], bestBSSID[4], bestBSSID[5], bestNetwork.ssid, bestChannel, bestNetworkDb);
	FixedString32 bestNetworkSsid = bestNetwork.ssid;
	ChangeState(WifiMultiState::Connecting, bestNetworkSsid);

	_logger.info(L("Connecting to '%s' network..."), bestNetworkSsid.c_str());

	status = WiFi.begin(bestNetwork.ssid, bestNetwork.passphrase, bestChannel, bestBSSID);
	return;
}

// ##################################################################################

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


bool ThingifyEspWiFiMulti::APlistAdd(const char* ssid, const char *passphrase) {

    WifiAPlist_t newAP;

    if(!ssid || *ssid == 0x00 || strlen(ssid) > 31) 
	{
        // fail SSID to long or missing!
		_logger.debug(LogComponent::Wifi, L("no ssid or ssid to long"));
        return false;
    }

    if(passphrase && strlen(passphrase) > 63) 
	{
        // fail passphrase to long!
		_logger.debug(LogComponent::Wifi, L("passphrase too long"));
        return false;
    }

    newAP.ssid = strdup(ssid);

    if(!newAP.ssid) {
		_logger.debug(LogComponent::Wifi, L("fail newAP.ssid == 0"));
        return false;
    }

    if(passphrase && *passphrase != 0x00) 
	{
        newAP.passphrase = strdup(passphrase);
        if(!newAP.passphrase) {
			_logger.debug(LogComponent::Wifi, L("fail newAP.passphrase == 0"));
            free(newAP.ssid);
            return false;
        }
    }

    APlist.push_back(newAP);   
    return true;
}

void ThingifyEspWiFiMulti::APlistClean(void) {
    for(uint32_t i = 0; i < APlist.size(); i++) 
	{
        WifiAPlist_t entry = APlist[i];
        if(entry.ssid) {
            free(entry.ssid);
        }
        if(entry.passphrase) {
            free(entry.passphrase);
        }
    }
    APlist.clear();
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

