#include "EspSmartConfig.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

bool EspSmartConfig::Start()
{
    _logger.info(L("EspSmartConfig::Start"));

    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    WiFi.stopSmartConfig();

    if(!WiFi.beginSmartConfig())
    {
          _logger.err(L("Begin smart config not successfull"));
          return false;
    }
    _smartConfigStartTime = millis();
    _state = SmartConfigState::WaitingForSmartConfigCredentials;
    return true;
}

bool EspSmartConfig::Stop()
{
    _logger.info(L("EspSmartConfig::Stop"));

    if(!WiFi.stopSmartConfig())
    {
        _logger.err(L("Waiting for smart config credentials ..."));
        return false;      
    }
    _state = SmartConfigState::NotRunning;
    return true;
}

SmartConfigState EspSmartConfig::Loop()
{
    #ifdef ESP8266

    if(_state == SmartConfigState::WaitingForSmartConfigCredentials)
    {
        if(WiFi.smartConfigDone())
        {
            struct station_config conf;
            wifi_station_get_config(&conf);
            _logger.info(L("Smart config received wifi details - SSID: %s Password: %s"), conf.ssid, conf.password);

            _state = SmartConfigState::ConnectingToWifi;
        }
    }

    if(_state == SmartConfigState::ConnectingToWifi)
    {
        if(WiFi.status() == WL_CONNECTED)
        {
            struct station_config conf;
            wifi_station_get_config(&conf);
            _logger.info(L("connected to WiFi, IP: %s"), WiFi.localIP().toString().c_str());
            _state = SmartConfigState::Success;
        }
    }

    #endif

    return _state;
}