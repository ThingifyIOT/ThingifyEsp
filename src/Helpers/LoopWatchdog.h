#ifndef _LOOP_WATCHDOG_H
#define _LOOP_WATCHDOG_H

#include "../Settings/SettingsStorage.h"

#ifdef ESP32
#include "esp32-hal-timer.h"
#endif
#include <stdint.h>

class LoopWatchdog
{
#ifdef ESP32
	hw_timer_t* timer = NULL;
#endif
public:
    static SettingsStorage* _settingsStorage;
	void Start(SettingsStorage* settingsStorage, uint64_t wdtTimeoutInMs);
	void Feed();
};


#endif