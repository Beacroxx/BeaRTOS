#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class SPI {
public:
  static void init();
  static void dmaTxCompleteCallback();
  static SPI_HandleTypeDef hspi4;
  static DMA_HandleTypeDef hdma_spi4_tx;
private:
  static void initDMA();
};

