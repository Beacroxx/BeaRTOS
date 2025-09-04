#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

namespace SPI {
void init();
void dmaTxCompleteCallback();
extern SPI_HandleTypeDef hspi4;
extern DMA_HandleTypeDef hdma_spi4_tx;

void initDMA();
} // namespace SPI
