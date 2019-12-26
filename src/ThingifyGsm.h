#ifndef _THINGIFY_GSM_H
#define _THINGIFY_GSM_H

#ifdef GSM

#include "Thingify.h"
#include <GsmModule.h>
#include <SimcomAtCommandsEsp32.h>
#include <HardwareSerial.h>
#include <stdint.h>
#include "Lib/EspAsyncTCPGsm/AsyncGsmClient.h"

struct ThingifyGsmP
{
	GsmModule _gsm;
	AsyncGsmClient _gsmClient;

	explicit ThingifyGsmP(uint8_t mux, SimcomAtCommands& simcomAtCommands):
		_gsm(simcomAtCommands),
		_gsmClient(_gsm, mux)
	{
		_gsm.ApnName = "virgin-internet";
		_gsm.BaudRate = 460800;
	}
};

class ThingifyGsm : 
	protected ThingifyGsmP,
	public Thingify
{
private:
	GsmState _previousGsmState;
	bool IsNetworkConnected();
	void OnFunctionExecutedByExternal(Node & node);
	void OnNodeValueChanedByExternal(Node& node);
	void HandleSleepMode();
	uint64_t _lastUserRequest = 0;
	bool _isPowerSavingActive = false;
	bool _isPowerSavingActivePrev = false;
	void StopNetwork() override;
	void StartNetwork() override;
public:
	ThingifyGsm(const char* deviceId, const char* deviceName, SimcomAtCommands& atCommands);

	void Start() override;
	void Loop() override;

	bool PowerSavingEnabled = false;
	int PowerSavingWakeupInterval = 2000;
	int PowerSavingSuspendTime = 10000;
	int ValueSendIntervalInSleepMode = 20000;
	int ValueSendInterval = 1000;
	bool GsmLogEnabled = true;
	bool GsmLogAtEnabled = true;
	RegistrationMode OperatorSelectionMode = RegistrationMode::Automatic;
	const char *NumericOperatorName = "";

	bool IsPowerSavingActive()
	{
		return _isPowerSavingActive;
	}
	GsmModule& Gsm()
	{
		return _gsm;
	}
	GsmAsyncSocket& Socket()
	{
		return _gsmClient.GsmSocket();
	}
	uint64_t WatchdogTimeoutInMs();
};

#endif
#endif
