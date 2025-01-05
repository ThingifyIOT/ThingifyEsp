#ifndef _RESET_SEQUENCE_DETECTOR_H
#define _RESET_SEQUENCE_DETECTOR_H

#include "Settings/SettingsStorage.h"
#include "Logging/Logger.h"


class ResetSequenceDetector
{
private:
    SettingsStorage _settingsStorage;
    unsigned long _timeout;
    bool _waitingForNextSequence;
    uint32_t doubleResetDetectorFlag;
    Logger& _logger = LoggerInstance;

    bool IsPowerOnReset()
    {
        #ifdef ESP32
        if(esp_reset_reason() == ESP_RST_POWERON)
        {
            return true;
        }
        return false;
        #elif ESP8266
        return true;
        #else
        return true;
        #endif
    }

public:
    ResetSequenceDetector(int timeout, SettingsStorage& settingsStorage): _settingsStorage(settingsStorage)
    {
        _timeout = timeout * 1000;
        _waitingForNextSequence = true;
    };

    bool IsResetSequenceDetected()
    {
        auto doubleResetFlag = _settingsStorage.GetDoubleResetFlag();

        switch (doubleResetFlag)
        {
        case RESET_SEQUENCE_CLEAR:
             _settingsStorage.SetDoubleResetFlag(RESET_SEQUENCE_FIRST_RESET);
            _logger.info(L("[CONFIG] first reset detected"));
            return false;
        case RESET_SEQUENCE_FIRST_RESET:
            _settingsStorage.SetDoubleResetFlag(RESET_SEQUENCE_SECOND_RESET);
            _logger.info(L("[CONFIG] second reset detected"));
            return false;
        case RESET_SEQUENCE_SECOND_RESET:
            _logger.info(L("[CONFIG] third reset detected"));
            _settingsStorage.SetDoubleResetFlag(RESET_SEQUENCE_CLEAR);
            return IsPowerOnReset();
        default:
            _logger.info(L("[CONFIG] unknown reset sequence flag"));
            return false;
        }
    };
   
    void Loop()
    {
        if (_waitingForNextSequence && millis() > _timeout)
        {
            _waitingForNextSequence = false;
            _settingsStorage.SetDoubleResetFlag(RESET_SEQUENCE_CLEAR);
            _logger.info(L("[CONFIG] Cleared config reset sequence"));
        }
    };
};
#endif