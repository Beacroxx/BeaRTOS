#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

namespace SystemClock {
void init();
void exitRun0Mode();
void configOscillator();
void configClock();
} // namespace SystemClock

extern "C" {
void ExitRun0Mode(void);
}