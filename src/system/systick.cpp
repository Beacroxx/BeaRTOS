#include "systick.hpp"

#include "system/clock.hpp"

// Initialize the system tick
void SystemTick::init() {
  SysTick->LOAD = (SystemCoreClock / 1000) - 1; // 1ms interval
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
  NVIC_SetPriority(SysTick_IRQn, 0);
  NVIC_SetPriority(SVCall_IRQn, 1); // lower so systick can cycle
  NVIC_SetPriorityGrouping(0);
  __enable_irq();
}

// System tick handler
void SystemTick::handler() { HAL_IncTick(); }