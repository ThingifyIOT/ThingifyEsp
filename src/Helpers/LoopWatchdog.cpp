#include "LoopWatchdog.h"
#include "../ContiUtils.h"
#include <FixedString.h>

#ifdef ESP32

void IRAM_ATTR resetModule() 
{
	FixedString20 restartReason;
	
	restartReason.append(F("WdtReset"));
	ContiUtils::WriteRestartReason(restartReason);
	Serial.println("WatchdogReset");
	esp_restart();
}
#endif


void LoopWatchdog::Start(uint64_t wdtTimeoutInMs)
{
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
