#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_tim.h"

class Timer {
public:
  static void init();
  static inline void irqHandler();

private:
  static TIM_HandleTypeDef htim2;
  static void initTimer2();
}; 