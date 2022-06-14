#include "SettingsStorage.h"
#include "EEPROM.h"
#include "../ThingifyConstants.h"

SettingsStorage::SettingsStorage()
{
}
bool SettingsStorage::_wasEepromInitialized = false;

void SettingsStorage::EnsureInitialized()
{
    if(_wasEepromInitialized)
    {
        return;
    }
    if(!EEPROM.begin(SettingsAddressEnd))
    {
        _logger.err(L("EEPROM.begin failed SettingsStorage::SettingsStorage"));
        return;
    }
    _wasEepromInitialized = true;
    _logger.info(L("EEPROM initialized"));
}
bool SettingsStorage::Set(ThingSettings &settings)
{
    EnsureInitialized();
    FixedString1024 data;
    if(!_serializer.Serialize(settings, data))
    {
        _logger.err(L("Failed to serialize settings"));
        return false;
    }

    uint16_t settingsCrc = Crc16(data.c_str(), data.length());
    uint16_t dataLength = data.length();
    _logger.info(L("settings length: %d, crc = 0x%04X"), data.length(), settingsCrc);

    
    if(!EepromWriteArray(SettingsAddressStart, (const char*)(&dataLength), 2))
    {
        return false;
    }
    if(!EepromWriteArray(SettingsAddressStart+2, (const char*)(&settingsCrc), 2))
    {
        return false;
    }
    if(!EepromWriteArray(SettingsAddressStart+4, data.c_str(), data.length()))
    {
        return false;
    }   
    
     print_data("data written to EEPROM", data.c_str(), data.length());

    if(!EEPROM.commit())
    {
        _logger.err(F("Failed to EEPROM.commit"));
        return false;
    }
    return true;
}

bool SettingsStorage::Get(ThingSettings &settings)
{
    EnsureInitialized();
    char dataLengthBytes[2];
    char dataCrcBytes[2];

    if(!EepromReadArray(SettingsAddressStart, dataLengthBytes, 2))
    {
        return false;
    }
    if(!EepromReadArray(SettingsAddressStart+2, dataCrcBytes, 2))
    {
        return false;
    }

    uint16_t dataLength = *(uint16_t*)&dataLengthBytes;
    uint16_t dataCrc = *(uint16_t*)&dataCrcBytes;

    if(dataLength == 0 && dataCrc == 0)
    {
        _logger.err(L("Settings are empty"));
        return false;
    }

    if(dataLength < 5 || dataLength > (SettingsAddressEnd - SettingsAddressStart - 4))
    {
        _logger.err(L("Incorrect data length: %d"), dataLength);
        return false;
    }
    FixedString1024 data;
    
    for(int i=0; i < dataLength; i++)
    {
        uint8_t byteFromEeprom = EEPROM.read(SettingsAddressStart+4+i);
        if(!data.append(byteFromEeprom))
        {
            Serial.println("Failed to move byte from eeprom to buffer");
        }
    }

    print_data("data from EEPROM", data.c_str(), data.length());
    uint16_t dataCrcCalculated = Crc16(data.c_str(), data.length());
    if(dataCrcCalculated != dataCrc)
    {
        _logger.err(L("Settings crc mismatch: 0x%02x(calculated) != 0x%02x(saved), dataLength: %d"), dataCrcCalculated, dataCrc, data.length());
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
    EnsureInitialized();
    EEPROM.begin(SettingsAddressEnd);
    EEPROM.write(SettingsAddressStart+4+5, 'x');
    EEPROM.commit();
}
void SettingsStorage::Clear()
{
    EnsureInitialized();
    for(int i=0; i <= SettingsAddressEnd; i++)
    {
        EEPROM.writeByte(i, 0);
    }
    EEPROM.commit();
}
bool SettingsStorage::EepromReadArray(int address, char* data, int length)
{
    EnsureInitialized();
    if(address + length > SettingsAddressEnd)
    {
        return false;
    }
    for(int i =0; i < length; i++)
    {
        data[i] = EEPROM.read(address+i);
    }
    return true;
}
bool SettingsStorage::EepromWriteArray(int address, const char* data, int length)
{
    if(address + length > SettingsAddressEnd)
    {
        return false;
    }
    for(int i =0; i < length; i++)
    {
        EEPROM.write(address+i, data[i]);
    }
    return true;
}

void SettingsStorage::print_data(const char* desc, const char* data_p, unsigned char dataLength)
{
    _logger.infoBegin(L("%s = ["), desc);
    for(int i=0; i < dataLength; i++)
    {
        _logger.add(L("%02X"), data_p[i]);
        if(i != dataLength - 1)
        {
            Serial.printf(", ");
        }
    }
    _logger.add(L("]"));
    _logger.end();
}

uint16_t SettingsStorage::Crc16(const char* data_p, unsigned char length)
{
    unsigned char x;
    uint16_t crc = 0xFFFF;

    while (length--)
    {
        x = crc >> 8 ^ *data_p++;
        x ^= x>> 4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
    }
    return crc;
}
void SettingsStorage::WriteRestartReason(FixedStringBase& errorStr)
{
    EnsureInitialized();
	auto maxLength = ThingifyConstants::MaxErrorStringLength;
	auto lengthToWrite = errorStr.length() > (maxLength-1) ? (maxLength-1) : errorStr.length();

	for (int i = 0; i < lengthToWrite; i++)
	{
		EEPROM.write(i, errorStr[i]);
	}
	EEPROM.write(lengthToWrite, 0);
	EEPROM.commit();
}


void SettingsStorage::ReadRestartReason(FixedStringBase& errorStr)
{
    EnsureInitialized();
	auto maxLength = ThingifyConstants::MaxErrorStringLength;
	errorStr.clear();

	for (int i = 0; i < maxLength; i++)
	{
		char c = EEPROM.read(i);
		if (c == 0)
		{
			break;
		}
		errorStr.append(c);
	}
}
void SettingsStorage::ClearRestartReason()
{
    EnsureInitialized();
	auto maxLength = ThingifyConstants::MaxErrorStringLength;

	for (int i = 0; i < maxLength; i++)
	{
		EEPROM.write(i, 0);
	}
	EEPROM.commit();
}
