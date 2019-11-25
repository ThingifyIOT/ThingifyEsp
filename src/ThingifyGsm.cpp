#include "ThingifyGsm.h"
#include <functional>

#ifdef GSM

using namespace std::placeholders;


ThingifyGsm::ThingifyGsm(const char* deviceId, const char *deviceName, SimcomAtCommands &atCommands):
	ThingifyGsmP(0, atCommands),
	Thingify(deviceId, deviceName, _gsmClient)
{
	_previousGsmState = _gsm.GetState();

	_gsm.At().Logger().OnLog([](const char *logEntry, bool flush)
	{
		if (flush)
		{
			Serial.flush();
			return;
		}
		auto time = millis();
		Serial.printf("%02lu.%03lu [GSM]: ", time / 1000, time % 1000);
		Serial.println(logEntry);
	});
	_firmwareUpdateService.SetMaxChunkSize(1024*10);
}

void ThingifyGsm::Start()
{
	Thingify::Start();
}

void ThingifyGsm::Loop()
{
	auto& gsmLogger = _gsm.At().Logger();

	gsmLogger.LogAtCommands = GsmLogAtEnabled;
	gsmLogger.LogEnabled = GsmLogEnabled;
	_gsm.OperatorSelectionMode = OperatorSelectionMode;
	_gsm.NumericOperatorName = NumericOperatorName;

	HandleSleepMode();

	_gsm.Loop();


	auto gsmState = _gsm.GetState();

	if (gsmState == GsmState::Error)
	{
		FixedString50 error;
		error.append(F("GSM:"));
		error.append(_gsm.Error());
		SetError(error.c_str());
	}

	if (gsmState != _previousGsmState)
	{
		if (gsmState == GsmState::ConnectingToGprs)
		{
			OnNetworkConnecting(_gsm.operatorName);
		}
		else if (gsmState == GsmState::ConnectedToGprs)
		{
			OnNetworkConnected();
		}
		else if (_previousGsmState == GsmState::ConnectedToGprs)
		{
			OnNetworkDisconnected();
		}
		_previousGsmState = gsmState;
	}
	Thingify::Loop();
}

bool ThingifyGsm::IsNetworkConnected()
{
	return _gsm.GetState() == GsmState::ConnectedToGprs;
}
void ThingifyGsm::OnFunctionExecutedByExternal(Node & node)
{
	_logger.info(F("Function executed by external"));
	_lastUserRequest = millis();

}
void ThingifyGsm::OnNodeValueChanedByExternal(Node & node)
{
	_logger.info(F("Node value changed by external"));
	_lastUserRequest = millis();
}

void ThingifyGsm::HandleSleepMode()
{
	auto now = millis();
	auto gsmState = _gsm.GetState();

	auto isPowerSavingSuspended =
		(now - _lastUserRequest < PowerSavingSuspendTime)
		|| (_firmwareUpdateService.IsUpdating())
		|| (now - GetConnectTime() < PowerSavingSuspendTime)
	    || (gsmState == GsmState::ConnectedToGprs && _gsm.GetTimeSinceStateChange() < PowerSavingSuspendTime)
		|| (gsmState != GsmState::ConnectedToGprs);

	_isPowerSavingActive = PowerSavingEnabled && !isPowerSavingSuspended;

	if (_isPowerSavingActive)
	{
		_gsm.TickInterval = PowerSavingWakeupInterval;
		_gsm.SleepEnabled = true;
		_gsm.GetTemperatureInterval = 15000;
		SetValueSendInterval(ValueSendIntervalInSleepMode);
	}
	else
	{
		_gsm.GetTemperatureInterval = 5000;
		_gsm.TickInterval = 100;
		_gsm.SleepEnabled = false;
		SetValueSendInterval(ValueSendInterval);
	}

	if (_isPowerSavingActive != _isPowerSavingActivePrev && !_gsm.At().IsInSleepMode())
	{
		auto enableNetlight = !_isPowerSavingActive;
		_logger.info(F("Setting netlight to %d"), enableNetlight);
		_gsm.At().EnableNetlight(enableNetlight);
	}
	_isPowerSavingActivePrev = _isPowerSavingActive;
}


uint64_t ThingifyGsm::WatchdogTimeoutInMs()
{
	return 30000;
}

#endif

