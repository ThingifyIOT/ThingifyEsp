#include "SettingsStorage.h"
#include "EEPROM.h"

bool SettingsStorage::Set(ThingSettings &settings)
{
    FixedString1024 data;
    if(!_serializer.Serialize(settings, data))
    {
        _logger.err(L("Failed to serialize settings"));
        return false;
    }

    uint16_t settingsCrc = Crc16(data.c_str(), data.length());
    uint16_t dataLength = data.length();
    _logger.info(L("settings length: %d, crc = 0x%04X"), data.length(), settingsCrc);

    EEPROM.begin(1092);
    EepromWriteArray(SettingsAddress, (const char*)(&dataLength), 2);
    EepromWriteArray(SettingsAddress+2, (const char*)(&settingsCrc), 2);
    EepromWriteArray(SettingsAddress+4, data.c_str(), data.length());
    if(!EEPROM.commit())
    {
        _logger.err(F("Failed to EEPROM.commit"));
        return false;
    }
    EEPROM.end();
    return true;
}

bool SettingsStorage::Get(ThingSettings &settings)
{
    EEPROM.begin(1092);
    char dataLengthBytes[2];
    char dataCrcBytes[2];

    EepromReadArray(SettingsAddress, dataLengthBytes, 2);
    EepromReadArray(SettingsAddress+2, dataCrcBytes, 2);

    uint16_t dataLength = *(uint16_t*)&dataLengthBytes;
    uint16_t dataCrc = *(uint16_t*)&dataCrcBytes;

    if(dataLength < 5 || dataLength > MaxSettingsSize)
    {
        _logger.err(L("Incorrect data length: %d\n"), dataLength);
        return false;
    }
    FixedString1024 data;
    for(int i=0; i < dataLength; i++)
    {
        data.append(EEPROM.read(SettingsAddress+4+i));
    }
    uint16_t dataCrcCalculated = Crc16(data.c_str(), data.length());
    if(dataCrcCalculated != dataCrc)
    {
        _logger.err(L("Settings crc mismatch: 0x%02x != 0x%02x"), dataCrcCalculated, dataCrc);
        return false;
    }

    if(!_serializer.Deserialize(data, settings))
    {
        _logger.err(L("Failed to deserialize settings"));
        return false;
    }
    return true;
}
bool SettingsStorage::BreakStorage()
{
    EEPROM.begin(1092);
    EEPROM.write(SettingsAddress+4+5, 'x');
    EEPROM.commit();
}
void SettingsStorage::Clear()
{
    EEPROM.begin(1092);
    for(int i=0; i < MaxSettingsSize + 4; i++)
    {
        EEPROM.write(SettingsAddress+i, 0);
    }
    EEPROM.commit();
}
bool SettingsStorage::EepromReadArray(int address, char* data, int length)
{
    for(int i =0; i < length; i++)
    {
        data[i] = EEPROM.read(address+i);
    }
}
bool SettingsStorage::EepromWriteArray(int address, const char* data, int length)
{
    for(int i =0; i < length; i++)
    {
        EEPROM.write(address+i, data[i]);
    }
}

uint16_t SettingsStorage::Crc16(const char* data_p, unsigned char length)
{
    unsigned char x;
    uint16_t crc = 0xFFFF;

    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}
