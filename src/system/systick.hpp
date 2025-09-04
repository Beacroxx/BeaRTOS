#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

namespace SystemTick {
void init();
void handler();
} // namespace SystemTick