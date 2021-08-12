#ifndef _SETTINGS_STORAGE_H
#define _SETTINGS_STORAGE_H

#include "ThingSettings.h"
#include "SettingsSerializer.h"
#include "Logging/Logger.h"

class SettingsStorage
{
    SettingsSerializer _serializer;
    Logger& _logger = LoggerInstance;
    uint16_t Crc16(const char* data_p, unsigned char length);
    bool EepromReadArray(int address, char* data, int length);
    bool EepromWriteArray(int address, const char* data, int length);
    const int SettingsAddress = 64;
    const int MaxSettingsSize = 1024;
public:
    bool BreakStorage();
    bool Set(ThingSettings &settings);
    bool Get(ThingSettings &settings);
    void Clear();
};

#endif

