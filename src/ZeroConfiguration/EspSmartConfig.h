#ifndef _ESP_SMART_CONFIG_H
#define _ESP_SMART_CONFIG_H

#include <stdint.h>
#include <Logging/Logger.h>

enum class SmartConfigState
{
    NotRunning,
    WaitingForSmartConfigCredentials,
    ConnectingToWifi,
    Success,
    Error
};

class EspSmartConfig
{
    uint64_t _smartConfigStartTime = 0;
    Logger& _logger = LoggerInstance;
    SmartConfigState _state;
public:
    bool Start();
    bool Stop();
    SmartConfigState Loop();
};

#endif