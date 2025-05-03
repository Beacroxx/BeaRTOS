#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class UART {
public:
  static void init();
  static void mspInit(UART_HandleTypeDef *huart);
  static int write(int fd, const char *buf, int count);
  static void dmaCallback();

  static UART_HandleTypeDef huart1;
  static DMA_HandleTypeDef hdma_usart1_tx;

private:
  static uint8_t *txBuffer;
  static bool dmaBusy;
}; 