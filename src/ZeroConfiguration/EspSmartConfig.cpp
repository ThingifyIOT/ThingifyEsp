#include "EspSmartConfig.h"
#include "FixedString.h"

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#else
#include <ESP8266WiFi.h>
#endif

bool EspSmartConfig::Start()
{
    _logger.info(L("EspSmartConfig::Start"));


    IPAddress localIp(192,168,1,1);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
    
    WiFi.softAPConfig(localIp, gateway, subnet);

    FixedString32 apName;
    apName.appendFormat(F("Thgfy-%d"), ESP.getChipId());
    if(!WiFi.softAP(apName.c_str(), "", 1, false, 10, 100))
    {
        _logger.info(L("Failed to start wifi ap"));
        return false;
    }
    _logger.info(L("Successfully started AP for smart config: %s"), apName.c_str());

    return true;
}

bool EspSmartConfig::Stop()
{
    _logger.info(L("EspSmartConfig::Stop"));

    WiFi.softAPdisconnect();
    _state = SmartConfigState::NotRunning;
    return true;
}

SmartConfigState EspSmartConfig::Loop()
{

    uint8_t wifiClientCount=WiFi.softAPgetStationNum(); 
    if(_currentWifiClientsCount<wifiClientCount)
    {
        _currentWifiClientsCount=wifiClientCount;
        _logger.info(L("New WiFi client connected, client count: %d"), _currentWifiClientsCount);
    }
    if(_currentWifiClientsCount>wifiClientCount)
    {
        _currentWifiClientsCount=wifiClientCount;
        _logger.info(L("WiFi client disconnected, client count: %d"), _currentWifiClientsCount);
    }
    return _state;
}