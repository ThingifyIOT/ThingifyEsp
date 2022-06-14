#include "EspSmartConfig.h"

#ifdef ESP32
#include <WiFi.h>
#include <esp_wifi.h>
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

    if(_state == SmartConfigState::WaitingForSmartConfigCredentials)
    {
        if(WiFi.smartConfigDone())
        {
            #ifdef ESP8266
            struct station_config conf;
            wifi_station_get_config(&conf);
            _logger.info(L("Smart config received wifi details - SSID: %s Password: %s"), conf.ssid, conf.password);
            #elif ESP32
            wifi_config_t wifi_config= {0};
            esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
            _logger.info(L("Smart config received wifi details - SSID: %s Password: %s"), wifi_config.sta.ssid, wifi_config.sta.password);
            #endif

            _state = SmartConfigState::ConnectingToWifi;
        }
    }

    if(_state == SmartConfigState::ConnectingToWifi)
    {
        if(WiFi.status() == WL_CONNECTED)
        {
            _logger.info(L("connected to WiFi, IP: %s"), WiFi.localIP().toString().c_str());
            _state = SmartConfigState::Success;
        }
    }

    return _state;
}