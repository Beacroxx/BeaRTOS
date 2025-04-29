#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class ErrorHandler {
public:
  static void handle();
  static void hardFault();
}; 