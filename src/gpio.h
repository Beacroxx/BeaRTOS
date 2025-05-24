#pragma once

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

namespace GPIO {
  extern const uint16_t LED_PIN;
  extern const uint32_t LED_PORT;

  void init();
  void toggleLed();
} 