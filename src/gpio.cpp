#include "main.h"

namespace GPIO {
const uint16_t LED_PIN = GPIO3;
const uint32_t LED_PORT = GPIOE;

void init() {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOE);

  // USART1
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10); // AF7 is USART1 on STM32H7

  // LED
  gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
  gpio_set(LED_PORT, LED_PIN);
}

void toggleLed() { gpio_toggle(LED_PORT, LED_PIN); }
} // namespace GPIO 