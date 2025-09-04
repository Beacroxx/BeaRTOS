#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"

namespace Timer {
void init();
extern TIM_HandleTypeDef htim1;
void initTimer1();
} // namespace Timer