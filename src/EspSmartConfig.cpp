#include "EspSmartConfig.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

bool EspSmartConfig::Start()
{
    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    WiFi.stopSmartConfig();
    if(!WiFi.beginSmartConfig())
    {
          Serial.println("Begin smart config not successfull");
          return false;
    }
    _isProcessingSmartConfig = true;
    _smartConfigStartTime = millis();

    return true;
}
bool EspSmartConfig::IsInProgress()
{
    return _isProcessingSmartConfig || _isConnectingToWifiFromSmartConfig;
}
bool EspSmartConfig::End()
{
    return true;
}

void EspSmartConfig::Loop()
{
    if(!IsInProgress())
    {
        return;
    }

    if(millis() - _smartConfigStartTime > 30000)
    {
        _isProcessingSmartConfig = false;
        return;
    }

    if(millis() - _lastSmartConfigIndication > 1000)
    {
        Serial.println("Waiting for smart config credentials ...");
        _lastSmartConfigIndication = millis();
    }
    
    #ifdef ESP8266

    if(WiFi.smartConfigDone() && !_isConnectingToWifiFromSmartConfig)
    {
        Serial.println("Smart config recevied credentials");
        struct station_config conf;
        wifi_station_get_config(&conf);
        Serial.printf("SSID: %s Password: %s\n", conf.ssid, conf.password);

        _isConnectingToWifiFromSmartConfig = true;
        _isProcessingSmartConfig = false;
    }

     if(_isConnectingToWifiFromSmartConfig && WiFi.status() == WL_CONNECTED)
    {
        struct station_config conf;
        wifi_station_get_config(&conf);
        Serial.printf("connected to WiFi, IP: %s\n", WiFi.localIP().toString().c_str());
        _isConnectingToWifiFromSmartConfig = false;
    }

    #endif
}