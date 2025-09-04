#pragma once

#if ENABLE_MICROSD

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_sd.h"

namespace MicroSD {
void init();
void readBlocks(uint8_t *pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
void writeBlocks(uint8_t *pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
uint64_t getCardInfo();
bool available();
extern SD_HandleTypeDef hsd;
extern bool isInitialized;
} // namespace MicroSD

#endif
