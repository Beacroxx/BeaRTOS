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
#include "peripherals/lcd.hpp"
#include "peripherals/spi.hpp"

#include <stdio.h>

extern "C" {

// SystemTick interrupt handler
void SysTick_Handler(void) {
  SystemTick::handler();
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  // Trigger PendSV interrupt
}

// HardFault interrupt handler
void HardFault_Handler(void) { ErrorHandler::hardFault(); }

// PendSV interrupt handler
void PendSV_Handler(void) {
  if (Scheduler::currentTask == nullptr) return; // If current task is nullptr, do nothing, no tasks to execute
  Scheduler::updateNextTask();
  Scheduler::switchTasks();
}

// UART MSP Init
void HAL_UART_MspInit(UART_HandleTypeDef *huart) { UART::mspInit(huart); }

// Redirect printf to UART
int _write(int fd, const char *buf, int count) { return UART::write(fd, buf, count); }

}

void task1(void) {
  while (1) {
    printf("Task 1\n");
    Scheduler::yieldDelay(1000);
  }
}

// Display task
void task2(void) {
  // determine max possible full frame fps by drawing a black screen 10 times
  uint32_t start = HAL_GetTick();
  for (int i = 0; i < 10; i++) {
    LCD::fillRect(0, 0, LCD::WIDTH, LCD::HEIGHT, BLACK);
  }
  uint32_t end = HAL_GetTick();
  uint32_t dt = (end - start) / 10;
  uint32_t id;
  LCD::readID(&id);
  char string[13];
  sprintf(string, "LCD ID: 0x%06lX", id);
  LCD::drawString(0, 0, 12, string);
  sprintf(string, "CPU: %lu MHz", HAL_RCC_GetSysClockFreq() / 1000000);
  LCD::drawString(0, 12, 12, string);
  // draw spi clock for the lcd
  PLL2_ClocksTypeDef PLL2_Clocks;
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clocks);
  uint32_t spi_freq = PLL2_Clocks.PLL2_Q_Frequency;
  sprintf(string, "SPI: %lu MHz", spi_freq / 1000000 / 2); // prescaler is 2
  LCD::drawString(0, 24, 12, string);
  sprintf(string, "Frame time: %lu ms (%lu Hz)", dt, 1000 / dt);
  LCD::drawString(0, 36, 12, string);

  while (1) {
    // draw the amount of tasks
    sprintf(string, "Tasks: %d", Scheduler::taskCount);
    LCD::drawString(0, 48, 12, string);
    Scheduler::yieldDelay(500);
  }
}

void task3(void) {
  while (1) {
    printf("Task 3\n");
    Scheduler::yieldDelay(1000);
    GPIO::toggleLed();
  }
}

void calledTask(void) {
  Scheduler::yieldDelay(1000);
  printf("Called task\n");
  while (1) {
    // Get system diagnostics
    printf("CPU Speed: %lu MHz\n", HAL_RCC_GetSysClockFreq() / 1000000);
    Scheduler::yieldDelay(5000);
  }
}

void exitingTask(void) {
  Scheduler::yieldDelay(500);
  Scheduler::initTaskStack(calledTask, 1024, "calledTask");
  Scheduler::yieldDelay(500);
  printf("Exiting task\n");
  return;
}

int main(void) {
  HAL_Init();
  SystemClock::init();
  SystemTick::init();
  GPIO::init();
  SPI::init();
  Timer::init();
  UART::init();
  LCD::init();

  Scheduler::init();
  Scheduler::initTaskStack(task1, 1024, "task1");
  Scheduler::initTaskStack(task2, 1024, "task2");
  Scheduler::initTaskStack(task3, 1024, "task3");
  Scheduler::initTaskStack(exitingTask, 1024, "exitingTask");
  Scheduler::start();

  while (1) {
    // Do nothing
  }
}