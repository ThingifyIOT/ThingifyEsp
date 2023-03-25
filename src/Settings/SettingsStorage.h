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
    
    // 0 - 3 double reset flag
    // 4-63 addresses are for restart reason string
    // 64-511 addresses are settings storage
    // 640-643 - double reset count

    const int SettingsAddressStart = 64;
    const int SettingsAddressEnd = 512;    

    const int InternalStatsMagicNumberAddress = 640;
    const int ResetSettingsCountAddress = 644;

    const int EEpromStorageEnd = 1024;


    void print_data(const char* desc, const char* data_p, unsigned char dataLength);
    static bool _wasEepromInitialized;
public:  
    SettingsStorage();
    void Initialize();
    bool BreakStorage();
    uint32_t GetDoubleResetFlag();
    void SetDoubleResetFlag(uint32_t doubleResetFlag);

    void SetInternalStatsMagicNumber(uint32_t magicNumber);
    uint32_t GetInternalStatsMagicNumber();

    void SetResetSettingsCount(uint32_t doubleResetCount);
    uint32_t GetResetSettingsCount();


    bool Set(ThingSettings &settings);
    bool Get(ThingSettings &settings);
    void Clear();
    void WriteRestartReason(FixedStringBase& errorStr);
	void ReadRestartReason(FixedStringBase& errorStr);
	void ClearRestartReason();
    
    static uint32_t InternalStatsStorageOkMagicNumber;
};
#endif

