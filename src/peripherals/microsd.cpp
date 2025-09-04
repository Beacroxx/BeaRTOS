#include "peripherals/microsd.hpp"

#include "error/handler.hpp"

#include <stdio.h>

#if ENABLE_MICROSD
namespace MicroSD {
SD_HandleTypeDef hsd;
bool isInitialized = false;
} // namespace MicroSD

void MicroSD::init() {
  // Reset initialization state
  isInitialized = false;

  // Configure GPIO pins
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_SDMMC1_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // Configure SDMMC pins
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // Configure NVIC
  HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

  // Initialize SDMMC peripheral
  hsd.Instance = SDMMC1;
  hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd.Init.ClockDiv = 4;

  // Initialize SD card
  if (HAL_SD_Init(&hsd) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SD_CARD_INIT_FAILED, __FILE__, __LINE__);
    isInitialized = false;
    return;
  }

  if (HAL_SD_ConfigWideBusOperation(&hsd, SDMMC_BUS_WIDE_4B) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SD_CARD_BUS_WIDTH_ERROR, __FILE__, __LINE__);
    isInitialized = false;
    return;
  }

  isInitialized = true;
}

void MicroSD::readBlocks(uint8_t *pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout) {
  if (!isInitialized)
    return;        // return silently if not initialized
  __disable_irq(); // ensure no interrupts during read
  if (HAL_SD_ReadBlocks(&hsd, pData, blockAddr, numOfBlocks, timeout) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SD_CARD_READ_FAILED, __FILE__, __LINE__);
  }
  __enable_irq();
}

void MicroSD::writeBlocks(uint8_t *pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout) {
  if (!isInitialized)
    return;        // return silently if not initialized
  __disable_irq(); // ensure no interrupts during write
  if (HAL_SD_WriteBlocks(&hsd, pData, blockAddr, numOfBlocks, timeout) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SD_CARD_WRITE_FAILED, __FILE__, __LINE__);
  }
  __enable_irq();
}

uint64_t MicroSD::getCardInfo() {
  if (!isInitialized)
    return 0; // return 0 if not initialized
  HAL_SD_CardInfoTypeDef cardInfo;
  if (HAL_SD_GetCardInfo(&hsd, &cardInfo) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SD_CARD_NOT_PRESENT, __FILE__, __LINE__);
  }
  uint64_t faketuple;
  faketuple = (uint64_t)cardInfo.BlockNbr << 32;
  faketuple |= (uint64_t)cardInfo.BlockSize;
  return faketuple;
}

bool MicroSD::available() { return isInitialized; }
#endif