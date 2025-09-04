#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#include <cstddef>
#include <deque>
#include <string>

namespace UART {
void init();
void mspInit(UART_HandleTypeDef *huart);
int write(const char *buf, int count);
void dmaCallback();

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern std::deque<std::string> txBuffers;
extern bool dmaBusy;
} // namespace UART