#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class SystemTick {
public:
  static void init();
  static void handler();
}; 