/*
 * MIT License
 * 
 * Copyright (c) 2024 Beacrox
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "error/handler.hpp"
#include "peripherals/gpio.hpp"
#include "peripherals/timer.hpp"
#include "peripherals/uart.hpp"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "system/clock.hpp"
#include "system/scheduler.hpp"
#include "system/systick.hpp"

#include <stdio.h>

extern "C" {
void SysTick_Handler(void) {
  SystemTick::handler();
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // Trigger PendSV interrupt
}

void TIM2_IRQHandler(void) { Timer::irqHandler(); }

void HardFault_Handler(void) { ErrorHandler::hardFault(); }

void PendSV_Handler(void) {
  Scheduler::switchTasks();
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) { UART::mspInit(huart); }

int _write(int fd, const char *buf, int count) { return UART::write(fd, buf, count); }
}


void task1(void) {
  while (1) {
    printf("Task 1\n");
    HAL_Delay(3000);
  }
}

void task2(void) {
  while (1) {
    printf("Task 2\n");
    HAL_Delay(2000);
  }
}

void task3(void) {
  while (1) {
    printf("Task 3\n");
    HAL_Delay(1000);
    GPIO::toggleLed();
  }
}

int main(void) {
  HAL_Init();
  SystemClock::init();
  SystemTick::init();
  GPIO::init();
  Timer::init();
  UART::init();

  Scheduler::init();
  Scheduler::initTaskStack(Scheduler::tasks[0], task1);
  Scheduler::initTaskStack(Scheduler::tasks[1], task2);
  Scheduler::initTaskStack(Scheduler::tasks[2], task3);
  Scheduler::start();

  while (1) {
    // Do nothing
  }
}