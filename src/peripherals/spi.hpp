#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class SPI {
public:
  static void init();
  static SPI_HandleTypeDef hspi4;
};

