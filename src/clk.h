#pragma once

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>

namespace CLK {
  extern uint32_t sysclk_frequency;
  void init();
} 