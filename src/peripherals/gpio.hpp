#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class GPIO {
public:
  static void init();
  static void toggleLed();
  static void setLed(bool state);
}; 