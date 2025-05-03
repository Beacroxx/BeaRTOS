#pragma once

#if ENABLE_MICROSD

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_sd.h"

class MicroSD {
  public:
    static void init();
    static void readBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
    static void writeBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
    static uint64_t getCardInfo();
    static bool available();
    static SD_HandleTypeDef hsd;

  private:
    static bool isInitialized;
};

#endif
