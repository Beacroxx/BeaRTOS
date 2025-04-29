#include "systick.hpp"

void SystemTick::init() {
  SysTick->LOAD = (SystemCoreClock / 1000) - 1; // 1ms interval
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
  NVIC_SetPriority(SysTick_IRQn, 1);
}

void SystemTick::handler() {
  HAL_IncTick();
} 