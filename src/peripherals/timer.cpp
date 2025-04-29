#include "timer.hpp"
#include "../peripherals/gpio.hpp"
#include "../error/handler.hpp"

TIM_HandleTypeDef Timer::htim2;

void Timer::init() {
  initTimer2();
}

void Timer::initTimer2() {
  __HAL_RCC_TIM2_CLK_ENABLE();
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 32500 - 1; // 325MHz / 32500 = 10kHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 5000 - 1; // 10kHz / 5000 = 2Hz (500ms)
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    ErrorHandler::handle();
  }

  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  HAL_TIM_Base_Start_IT(&htim2);
}

void Timer::irqHandler() {
  HAL_TIM_IRQHandler(&htim2);
} 