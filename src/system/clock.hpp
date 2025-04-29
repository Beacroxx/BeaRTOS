#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class SystemClock {
public:
  static void init();
  static void exitRun0Mode();

private:
  static void configOscillator();
  static void configClock();
};

extern "C" {
  void ExitRun0Mode(void);
} 