﻿#ifndef _FIRMWARE_UPDATE_SERVICE_H
#define _FIRMWARE_UPDATE_SERVICE_H

#include "PacketSender.h"
#include "Api/UpdateFirmwareBeginToThingPacket.h"
#include "Api/UpdateFirmwareCommitToThingPacket.h"
#include "Api/UpdateFirmwareDataAck.h"
#include "Logging/Logger.h"
#include "ContiUtils.h"
#include <stdint.h>

class FirmwareUpdateService
{
private:
	uint16_t _lastFirmwareCorrelationId;
	PacketSender& _packetSender;
	Logger& _logger;
	FixedString100 _errorString;
	bool _isSuccess;
	uint32_t _maxFirmwareChunkSize;
	bool Begin(uint64_t size);
	bool SetExpectedMd5(const char* md5);
	bool WriteData(const char* data, int length);
	bool Commit();	
	FixedString100 GetLastError();
	bool _isUpdatingFirmware;
	static const __FlashStringHelper* UploadErrorToString(uint8_t error);
	ElapsedTimer _restartRequestedTimer;
	ElapsedTimer _dataTimeoutTimer;
	uint8_t _updateProgress;
public:
	FirmwareUpdateService(PacketSender& packetSender);
	void SetMaxChunkSize(uint32_t maxChunkSize);
	bool IsUpdating();
	uint8_t UpdateProgress();
	void HandleFirmwareData(uint32_t correlationId, uint64_t firmwareDataIndex, const char* data, size_t index, size_t length, size_t totalPayloadLength);
	void HandleUpdateFirmwareBegin(UpdateFirmwareBeginToThingPacket* packet);
	void HandleFirmwareCommitPacket(UpdateFirmwareCommitToThingPacket* packet);
	void Loop();

	static bool ParseNumber(const char* str, uint16_t length, uint64_t& number);
	static bool ParseFirmwareTopic(const char* topic, uint32_t& correlationId, uint64_t& dataOffset);

};

#endif


