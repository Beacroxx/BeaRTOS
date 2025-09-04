#include "spi.hpp"

#include "../error/handler.hpp"
#include "lcd.hpp"

namespace SPI {
SPI_HandleTypeDef hspi4;
DMA_HandleTypeDef hdma_spi4_tx;
} // namespace SPI

void SPI::init() {
  __HAL_RCC_SPI4_CLK_ENABLE();

  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_1LINE;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 0x0;
  hspi4.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;

  if (HAL_SPI_Init(&hspi4) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::SPI_INIT_FAILED);
  }

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (hspi4.Instance == SPI4) {
    __HAL_RCC_SPI4_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI4;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  }

  initDMA();
}

void SPI::initDMA() {

  hdma_spi4_tx.Instance = DMA1_Stream0;
  hdma_spi4_tx.Init.Request = DMA_REQUEST_SPI4_TX;
  hdma_spi4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_spi4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_spi4_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_spi4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_spi4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_spi4_tx.Init.Mode = DMA_NORMAL;
  hdma_spi4_tx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_spi4_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_spi4_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_spi4_tx.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_spi4_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;

  if (HAL_DMA_Init(&hdma_spi4_tx) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::DMA_INIT_FAILED, __FILE__, __LINE__);
  }

  __HAL_LINKDMA(&hspi4, hdmatx, hdma_spi4_tx);

  // Enable DMA interrupts
  __HAL_DMA_ENABLE_IT(&hdma_spi4_tx, DMA_IT_TC);

  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

  HAL_NVIC_SetPriority(SPI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SPI4_IRQn);
}

void SPI::dmaTxCompleteCallback() {
#if ENABLE_LCD
  LCD_CS_SET; // Release CS after DMA transfer
  LCD::dma_busy = false;
#endif
}