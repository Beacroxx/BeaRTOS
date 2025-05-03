#include "uart.hpp"
#include "../error/handler.hpp"
#include "../system/scheduler.hpp"
#include "../system/memory.hpp"
#include <cstring>

UART_HandleTypeDef UART::huart1;
DMA_HandleTypeDef UART::hdma_usart1_tx;
uint8_t *UART::txBuffer = nullptr;
bool UART::dmaBusy = false;

void UART::init() {
  // Initialize DMA controller
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_DMA_RESET_HANDLE_STATE(&hdma_usart1_tx);

  // Configure UART
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 1500000; // 1.5 Mbaud
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&huart1) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::UART_INIT_FAILED, __FILE__, __LINE__);
  }

  // Configure DMA
  hdma_usart1_tx.Instance = DMA1_Stream5;
  hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
  hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart1_tx.Init.Mode = DMA_NORMAL;
  hdma_usart1_tx.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_usart1_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_usart1_tx.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_usart1_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;

  if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::DMA_INIT_FAILED, __FILE__, __LINE__);
  }

  __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void UART::mspInit(UART_HandleTypeDef *huart) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (huart->Instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

int UART::write(int fd, const char *buf, int count) {
  if (count <= 0) return 0;

  // busy wait for any ongoing transfer to complete
  while (dmaBusy) {
    asm volatile("nop");
  }

  dmaBusy = true;

  txBuffer = static_cast<uint8_t *>(Memory::realloc(txBuffer, count, __FILE__, __LINE__));
  memcpy(txBuffer, buf, count);

  if (HAL_UART_Transmit_DMA(&huart1, txBuffer, count) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::UART_TRANSMIT_FAILED, __FILE__, __LINE__);
    return -1;
  }
  return count;
} 

void UART::dmaCallback() {
  dmaBusy = false;
}