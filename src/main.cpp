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
#include "system/memory.hpp"
#include "peripherals/microsd.hpp"
#include "peripherals/adc.hpp"

#include <stdio.h>
#include <string.h>

extern "C" {

// SystemTick interrupt handler
void SysTick_Handler(void) {
  SystemTick::handler();
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

// HardFault interrupt handler
void HardFault_Handler(void) { 
  ErrorHandler::hardFault(ErrorCode::HARD_FAULT, __FILE__, __LINE__); 
}

// PendSV interrupt handler
void PendSV_Handler(void) {
  if (!Scheduler::active) return; // If scheduler is not active, do nothing, no tasks to execute
  Scheduler::updateNextTask();
  Scheduler::switchTasks();
}

// UART MSP Init
void HAL_UART_MspInit(UART_HandleTypeDef *huart) { UART::mspInit(huart); }

// Redirect printf to UART
int _write(int fd, const char *buf, int count) { return UART::write(fd, buf, count); }

// DMA interrupt handler
void DMA1_Stream0_IRQHandler(void) {
  HAL_DMA_IRQHandler(&SPI::hdma_spi4_tx);
}

void SPI4_IRQHandler(void) {
  HAL_SPI_IRQHandler(&SPI::hspi4);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi->Instance == SPI4) {
    SPI::dmaTxCompleteCallback();
  }
}

}

void Init_MPU() {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  HAL_MPU_Disable();

  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x24000000; // AXI SRAM base
  MPU_InitStruct.Size = MPU_REGION_SIZE_1MB; // Adjust as needed
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE; // Write-through
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

// SD card task
void task1(void) {
  if (MicroSD::available()) {
    uint64_t cardInfo = MicroSD::getCardInfo();
    uint32_t cardSize = cardInfo >> 32;
    uint32_t cardBlockSize = cardInfo & 0xFFFFFFFF;
    uint64_t totalBytes = (uint64_t)cardSize * (uint64_t)cardBlockSize;
    uint32_t totalMB = totalBytes / (1024 * 1024);
    printf("Card size: %lu blocks, %lu MB\n", cardSize, totalMB);
  } else {
    printf("Card not available\n");
  }

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
    LCD::update();
  }
  uint32_t end = HAL_GetTick();
  uint32_t dt = (end - start) / 10;
  // draw spi clock for the lcd
  PLL2_ClocksTypeDef PLL2_Clocks;
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clocks);
  uint32_t spi_freq = PLL2_Clocks.PLL2_Q_Frequency;
  char string[32];

  while (1) {
    // Get memory statistics
    Memory::MemoryRegion flash, ram;
    uint32_t heapUsed, heapFree;
    Memory::getStats(flash, ram, heapUsed, heapFree);

    // Draw stats
    sprintf(string, "CPU: %lu MHz, SPI: %lu MHz  ", HAL_RCC_GetSysClockFreq() / 1000000, spi_freq / 1000000 / 2); // prescaler is 2
    LCD::drawString(0, 0, 12, string);
    sprintf(string, "Frame time: %lu ms (%lu Hz)  ", dt, 1000 / dt);
    LCD::drawString(0, 12, 12, string);
    LCD::update();
    sprintf(string, "Flash: %lu / %lu KB  ", flash.used / 1024, flash.size / 1024);
    LCD::drawString(0, 24, 12, string);
    sprintf(string, "RAM: %lu / %lu KB  ", (ram.used + heapUsed) / 1024, (ram.size + heapFree) / 1024);
    LCD::drawString(0, 36, 12, string);
    sprintf(string, "Tasks: %d  ", Scheduler::taskCount);
    LCD::drawString(0, 48, 12, string);
    float temp = ADC::getTemperature();
    sprintf(string, "Core Temp: %d.%d C    ", (uint32_t)temp, (uint32_t)(temp * 10) % 10);
    LCD::drawString(0, 60, 12, string);
    LCD::update();
    printf("Task 2\n");
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
  printf("called task\n");
  while (1) {
    // Get system diagnostics
    printf("called task\n");
    Scheduler::yieldDelay(500);
  }
}

void exitingTask(void) {
  printf("Exiting task starting\n");
  Scheduler::yieldDelay(500);
  Scheduler::initTaskStack(calledTask, 128, "calledTask");
  Scheduler::yieldDelay(500);
  printf("Exiting task done\n");
}

int main(void) {
  if (HAL_Init() != HAL_OK) {
    ErrorHandler::handle(ErrorCode::HAL_INIT_FAILED, __FILE__, __LINE__);
  }
  SCB_EnableICache();
  SCB_EnableDCache();
  Init_MPU();
  SystemClock::init();
  SystemTick::init();
  GPIO::init();
  UART::init();
  Memory::init();
  SPI::init();
  Timer::init();
  MicroSD::init();
  LCD::init();

  // enable temperature sensor
  ADC3_COMMON->CCR |= ADC_CCR_TSEN;
  HAL_Delay(10); // Wait at least 10µs for sensor to stabilize

  ADC::init();
  ADC::calibrate();

  printf("Initializing tasks\n");

  Scheduler::initTaskStack(task1, 128, "task1");
  Scheduler::initTaskStack(task2, 128, "task2");
  Scheduler::initTaskStack(task3, 128, "task3");
  Scheduler::initTaskStack(exitingTask, 128, "exitingTask");

  Scheduler::start();

  // will not get here ideally
  ErrorHandler::handle(ErrorCode::UNKNOWN, __FILE__, __LINE__);
}