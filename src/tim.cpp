#include "main.h"

namespace TIM {
const uint32_t SYSTICK_FREQ = 1000; // 1kHz
volatile uint64_t systick_ms = 0;

void init() {
  // Systick
  systick_set_frequency(SYSTICK_FREQ, CLK::sysclk_frequency); // PLL1P is source
  systick_counter_enable();
  systick_interrupt_enable();
}

// get Milliseconds since boot
uint64_t get_ms() { return systick_ms; }

// get Microseconds since boot
uint64_t get_us() {
  constexpr uint64_t ticks_per_us = 550;
  constexpr uint64_t ticks_per_ms = 550000;

  uint64_t ms1, ms2, val;
  do {
    ms1 = get_ms();
    val = systick_get_value();
    ms2 = get_ms();
  } while (ms1 != ms2);

  return ms1 * 1000 + (ticks_per_ms - val) / ticks_per_us;
}

void delay_us(uint64_t us) {
  uint64_t start = get_us();
  while (get_us() - start < us) {
    yield();
  }
}

// NOTE: delay_ms is a wrapper around delay_us
void delay_ms(uint64_t ms) {
  delay_us(ms * 1000);
}
} // namespace TIM 

extern "C" {
// SysTick ISR
void sys_tick_handler(void) {
  TIM::systick_ms++;
  yield();
}
}