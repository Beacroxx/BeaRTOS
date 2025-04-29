#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

class UART {
public:
  static void init();
  static void mspInit(UART_HandleTypeDef *huart);
  static int write(int fd, const char *buf, int count);

private:
  static UART_HandleTypeDef huart1;
}; 