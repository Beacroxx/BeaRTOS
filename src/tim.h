#pragma once

#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/cortex.h>

namespace TIM {
  extern const uint32_t SYSTICK_FREQ;
  extern volatile uint64_t systick_ms;

  void init();
  uint64_t get_ms();
  uint64_t get_us();
  void delay_us(uint64_t us);
  void delay_ms(uint64_t ms);
} 