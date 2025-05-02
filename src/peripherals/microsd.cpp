#include "peripherals/microsd.hpp"
#include "error/handler.hpp"
#include <stdio.h>

SD_HandleTypeDef MicroSD::hsd;

void MicroSD::init() {
  // Configure GPIO pins
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_SDMMC1_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(SDMMC1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

  // Initialize SDMMC peripheral
  hsd.Instance = SDMMC1;
  hsd.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 5;

	__HAL_RCC_SDMMC1_FORCE_RESET();
	__HAL_RCC_SDMMC1_RELEASE_RESET();

  if (HAL_SD_Init(&hsd) != HAL_OK) {
    ErrorHandler::handle();
  }

	// set 4 bit mode
	if (HAL_SD_ConfigWideBusOperation(&hsd, SDMMC_BUS_WIDE_4B) != HAL_OK) {
	}
}

void MicroSD::readBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout) {
  if (HAL_SD_ReadBlocks(&hsd, pData, blockAddr, numOfBlocks, timeout) != HAL_OK) {
    ErrorHandler::handle();
  }
}

void MicroSD::writeBlocks(uint8_t* pData, uint32_t blockAddr, uint32_t numOfBlocks, uint32_t timeout) {
  if (HAL_SD_WriteBlocks(&hsd, pData, blockAddr, numOfBlocks, timeout) != HAL_OK) {
    ErrorHandler::handle();
  }
}

uint64_t MicroSD::getCardInfo() {
  HAL_SD_CardInfoTypeDef cardInfo;
  if (HAL_SD_GetCardInfo(&hsd, &cardInfo) != HAL_OK) {
    ErrorHandler::handle();
  }
	uint64_t faketuple;
	faketuple = (uint64_t)cardInfo.BlockNbr << 32;
	faketuple |= (uint64_t)cardInfo.BlockSize;
  return faketuple;
} 