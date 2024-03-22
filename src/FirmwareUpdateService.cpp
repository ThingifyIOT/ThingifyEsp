#include "FirmwareUpdateService.h"
#include <string.h>

#ifdef ESP32
#include <Update.h>
#elif defined(ESP8266)
#include <Updater.h>
#endif // ESP32


FirmwareUpdateService::FirmwareUpdateService(PacketSender& packetSender, SettingsStorage& settingsStorage) :
	_lastFirmwareCorrelationId(0),
	_packetSender(packetSender),
	_logger(LoggerInstance),
	_maxFirmwareChunkSize(50*1024),
	_isUpdatingFirmware(false),
	_updateProgress(0),
	_settingsStorage(settingsStorage)
{
#ifdef ESP32
	_maxFirmwareChunkSize = 150 * 1024;
#endif // ESP32

}

void FirmwareUpdateService::SetMaxChunkSize(uint32_t maxChunkSize)
{
	_maxFirmwareChunkSize = maxChunkSize;
}

bool FirmwareUpdateService::IsUpdating()
{
	return _isUpdatingFirmware;
}

uint8_t FirmwareUpdateService::UpdateProgress()
{
	return _updateProgress;
}

void FirmwareUpdateService::HandleUpdateFirmwareBegin(UpdateFirmwareBeginToThingPacket* packet)
{
	_errorString = "";
	_isSuccess = true;
	_updateProgress = 0;

	_settingsBackup = new ThingSettings();
	if(!_settingsStorage.Get(*_settingsBackup))
	{
		delete _settingsBackup;
		_settingsBackup = nullptr;
	}

	_logger.info(L("Update firmware begin: md5 = %s, size = %lld"), packet->FirmwareMd5.c_str(), packet->FirmwareSize);
	if (!Begin(packet->FirmwareSize))
	{
		_isSuccess = false;
		_errorString = GetLastError();
		_logger.err(F("Firmware begin failed: %s"), _errorString.c_str());
		_lastFirmwareCorrelationId = packet->CorrelationId;
		return;		
	}
	if (!SetExpectedMd5(packet->FirmwareMd5.c_str()))
	{
		_isSuccess = false;
		_errorString = F("Failed to set md5");
		_logger.err(F("Firmware begin failed: %s"), _errorString.c_str());
		_lastFirmwareCorrelationId = packet->CorrelationId;
		return;
	}
	_logger.info(F("Firmware begin update success"));
	_isUpdatingFirmware = true;
	_lastFirmwareCorrelationId = packet->CorrelationId;
	_dataTimeoutTimer.Start();
}

void FirmwareUpdateService::HandleFirmwareData(uint32_t correlationId, uint64_t firmwareDataIndex, const char* data, size_t index, size_t length, size_t totalPayloadLength)
{

	if (!_isSuccess)
	{
		return;
	}
	if (!WriteData(data, length))
	{
		_isSuccess = false;
		_isUpdatingFirmware = false;
		_errorString = GetLastError();
		_dataTimeoutTimer.Stop();
		_logger.err(L("Failed to write firmware data: %s"), _errorString.c_str());
		_lastFirmwareCorrelationId = correlationId;
		return;
	}
	_dataTimeoutTimer.Start();

	if (Update.size() > 0)
	{
		_updateProgress = Update.progress() * 100 / Update.size();
	}
	if (totalPayloadLength == index + length)
	{
		_lastFirmwareCorrelationId = correlationId;
	}
}

void FirmwareUpdateService::HandleFirmwareCommitPacket(UpdateFirmwareCommitToThingPacket* packet)
{
	_dataTimeoutTimer.Stop();

	if (!_isSuccess)
	{
		_isUpdatingFirmware = false;
		return;
	}
	_logger.info(L("Commiting firmware..."));
	if (!Commit())
	{
		_isUpdatingFirmware = false;
		_isSuccess = false;
		_errorString = GetLastError();
		_logger.err(L("Firmware commit failed: %s"), _errorString.c_str());
		_lastFirmwareCorrelationId = packet->CorrelationId;
		return;
	}

	if(_settingsBackup != nullptr)
	{	
		_logger.info(L("Restoring settings..."));
		_settingsStorage.Set(*_settingsBackup);
		delete _settingsBackup;
		_settingsBackup = nullptr;
	}
	else
	{
		_logger.info(L("Skip restore settings as they are not available"));
	}

	_lastFirmwareCorrelationId = packet->CorrelationId;
	_logger.info(L("Firmware commit success!"));
	_restartRequestedTimer.Start();
}

bool FirmwareUpdateService::Begin(uint64_t size)
{
#ifdef ESP32
	Update.abort();
#endif

#ifdef ESP8266
	Update.end();
	Update.runAsync(true);
#endif

	return Update.begin(size);
}

bool FirmwareUpdateService::SetExpectedMd5(const char* md5)
{
	return Update.setMD5(md5);
}

FixedString128 FirmwareUpdateService::GetLastError()
{
	FixedString128 error;
	error = UploadErrorToString(Update.getError());
	return error;
}

bool FirmwareUpdateService::WriteData(const char* data, int length)
{
	if (Update.write(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data)), length) != length)
	{
		return false;
	}
	return true;
}

bool FirmwareUpdateService::Commit()
{
	return Update.end(true);
}

void FirmwareUpdateService::Loop()
{
	if (_lastFirmwareCorrelationId != 0)
	{
		auto correlationId = _lastFirmwareCorrelationId;
		_lastFirmwareCorrelationId = 0;

		const auto updateFirmwareDataAck = new UpdateFirmwareDataAck();
		updateFirmwareDataAck->CorrelationId = correlationId;
		updateFirmwareDataAck->IsSuccess = _isSuccess;
		updateFirmwareDataAck->ErrorString = _errorString;
		updateFirmwareDataAck->MaxChunkSize = _maxFirmwareChunkSize;
		_packetSender.SendPacket(updateFirmwareDataAck);
		delete updateFirmwareDataAck;
	}

	if(_dataTimeoutTimer.IsStarted() && _dataTimeoutTimer.ElapsedSeconds() > 35)
	{
		_dataTimeoutTimer.Stop();
		_isUpdatingFirmware = false;
		_logger.err(F("Timeout out waiting for next firmware data"));
	}

	if (_restartRequestedTimer.IsStarted() && _restartRequestedTimer.ElapsedMs() > 1000)
	{
		_logger.info(L("Restarting thing after firmware update..."));

		FixedString32 restartReason = "FirmwareUpdate";
        _settingsStorage.WriteRestartReason(restartReason);
		delay(100);
		ThingifyUtils::RestartDevice();
	}
}

const __FlashStringHelper* FirmwareUpdateService::UploadErrorToString(uint8_t error)
{
#ifdef ESP32
	switch (error)
	{
		case UPDATE_ERROR_OK:	 return F("No Error");
		case UPDATE_ERROR_WRITE: return F("Flash Write Failed");
		case UPDATE_ERROR_ERASE: return F("Flash Erase Failed");
		case UPDATE_ERROR_READ:  return F("Flash Read Failed");
		case UPDATE_ERROR_SPACE: return F("Not Enough Space");
		case UPDATE_ERROR_SIZE:	 return F("Bad Size Given");
		case UPDATE_ERROR_STREAM: return F("Stream Read Timeout");
		case UPDATE_ERROR_MD5:	 return F("MD5 Check Failed");
		case UPDATE_ERROR_MAGIC_BYTE:	return F("Wrong Magic Byte");
		case UPDATE_ERROR_ACTIVATE:		return F("Could Not Activate The Firmware");
		case UPDATE_ERROR_NO_PARTITION:	return F("Partition Could Not be Found");
		case UPDATE_ERROR_BAD_ARGUMENT:	return F("Bad Argument");
		case UPDATE_ERROR_ABORT:		return F("Aborted");
		default: return F("UNKNOWN");
	}
#elif defined(ESP8266)
	switch (error)
	{
		case UPDATE_ERROR_OK: return F("No Error");		
		case UPDATE_ERROR_WRITE: return F("Flash Write Failed");		
		case UPDATE_ERROR_ERASE: return F("Flash Erase Failed");		
		case UPDATE_ERROR_READ: return F("Flash Read Failed");		
		case UPDATE_ERROR_SPACE: return F("Not Enough Space");		
		case UPDATE_ERROR_SIZE: return F("Bad Size Given");		
		case UPDATE_ERROR_STREAM: return F("Stream Read Timeout");		
		case UPDATE_ERROR_MD5: return F("MD5 check failed");		
		case UPDATE_ERROR_SIGN: return F("Signature verification failed");		
		case UPDATE_ERROR_FLASH_CONFIG: return F("Flash config wrong");		
		case UPDATE_ERROR_NEW_FLASH_CONFIG: return F("new Flash config wrong");		
		case UPDATE_ERROR_MAGIC_BYTE: return F("Magic byte is wrong, not 0xE9");		
		case UPDATE_ERROR_BOOTSTRAP: return F("Invalid bootstrapping state, reset ESP8266 before updating");
		default: return F("UNKNOWN");		
	}
#endif
}

bool FirmwareUpdateService::ParseNumber(const char* str, uint16_t length, uint64_t& number)
{
	number = 0;
	uint64_t multiplexer = 1;
	if (length == 0)
	{
		return false;
	}
	do
	{
		length--;
		const auto c = str[length];
		if (c < '0' || c > '9')
		{
			return false;
		}
		const auto digit = c - '0';
		number += digit * multiplexer;
		multiplexer *= 10;
	} while (length > 0);
	return true;
}
bool FirmwareUpdateService::ParseFirmwareTopic(const char* topic, uint32_t& correlationId, uint64_t& dataOffset)
{
	const char* firmwareSubtopic = "firmware/";
	const char* firmwarePacketInfo = strstr(topic, firmwareSubtopic);
	if (firmwarePacketInfo == nullptr)
	{
		return false;
	}
	firmwarePacketInfo += strlen(firmwareSubtopic);
	auto separatorPtr = strchr(firmwarePacketInfo, '_');
	if (separatorPtr == nullptr)
	{
		return false;
	}
	uint64_t correlationIdTmp = 0;
	if (!ParseNumber(firmwarePacketInfo, separatorPtr - firmwarePacketInfo, correlationIdTmp))
	{
		return false;
	}
	const auto totalLength = strlen(firmwarePacketInfo);
	const auto secondTokenLength = totalLength - (separatorPtr - firmwarePacketInfo) - 1;
	if (!ParseNumber(separatorPtr + 1, secondTokenLength, dataOffset))
	{
		return false;
	}
	correlationId = correlationIdTmp;
	return true;
}

