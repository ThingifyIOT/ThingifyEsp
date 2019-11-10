#ifndef _ESP_SMART_CONFIG_H
#define _ESP_SMART_CONFIG_H

#include <stdint.h>

class EspSmartConfig
{
    bool _isProcessingSmartConfig = false;
    bool _isConnectingToWifiFromSmartConfig = false;
    uint64_t _lastSmartConfigIndication = 0;
    uint64_t _smartConfigStartTime = 0;
public:
    bool Start();
    bool IsInProgress();
    bool End();
    void Loop();
};

#endif