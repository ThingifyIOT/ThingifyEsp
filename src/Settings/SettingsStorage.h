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
    // 0-63 addresses are for restart reason string
    // 64-512 addresses are settings storage
    const int SettingsAddressStart = 64;
    const int SettingsAddressEnd = 512;
    
    void print_data(const char* desc, const char* data_p, unsigned char dataLength);
    static bool _wasEepromInitialized;
    void EnsureInitialized();
public:
    SettingsStorage();
    bool BreakStorage();
    bool Set(ThingSettings &settings);
    bool Get(ThingSettings &settings);
    void Clear();
    void WriteRestartReason(FixedStringBase& errorStr);
	void ReadRestartReason(FixedStringBase& errorStr);
	void ClearRestartReason();

};

#endif

