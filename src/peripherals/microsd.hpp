#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_sd.h"

class MicroSD {
  public:
    static void init();
    static void readBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
    static void writeBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout);
    static uint64_t getCardInfo();
    static SD_HandleTypeDef hsd;

  private:
};
