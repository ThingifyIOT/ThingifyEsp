#include "LoopWatchdog.h"
#include "../ThingifyUtils.h"
#include <FixedString.h>

#ifdef ESP32

void IRAM_ATTR resetModule() 
{
	FixedString32 restartReason;
	
	restartReason.append(F("WdtReset"));
    if(LoopWatchdog::_settingsStorage != nullptr)
    {
        LoopWatchdog::_settingsStorage->WriteRestartReason(restartReason);
    }
	Serial.println("Thingify::WatchdogReset");
	esp_restart();
}
#endif


void LoopWatchdog::Start(SettingsStorage* settingsStorage, uint64_t wdtTimeoutInMs)
{
     _settingsStorage = settingsStorage;
#ifdef ESP32
	timer = timerBegin(0, 80, true);                  //timer 0, div 80
	timerAttachInterrupt(timer, &resetModule, true);  //attach callback
	timerAlarmWrite(timer, wdtTimeoutInMs * 1000, false); //set time in us
	timerAlarmEnable(timer);                          //enable interrupt
#endif
}
void LoopWatchdog::Feed()
{
#ifdef ESP32
	timerWrite(timer, 0);
#endif

}

SettingsStorage* LoopWatchdog::_settingsStorage = nullptr;
