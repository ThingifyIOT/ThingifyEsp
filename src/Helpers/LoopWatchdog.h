#ifndef _LOOP_WATCHDOG_H
#define _LOOP_WATCHDOG_H

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
	void Start(uint64_t wdtTimeoutInMs);
	void Feed();
};

#endif