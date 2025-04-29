#include "gpio.hpp"

void GPIO::init() {
  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  setLed(true);
}

void GPIO::toggleLed() {
  HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
}

void GPIO::setLed(bool state) {
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
} 