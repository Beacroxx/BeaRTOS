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

// PendSV exception handler
void PendSV_Handler(void) {
  if (!Scheduler::active) return; // If scheduler is not active, do nothing, no tasks to execute
  Scheduler::updateNextTask();
  Scheduler::switchTasks();
}

// UART MSP Init
void HAL_UART_MspInit(UART_HandleTypeDef *huart) { UART::mspInit(huart); }

// Redirect printf to UART
int _write(int fd, const char *buf, int count) { return UART::write(fd, buf, count); }

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    UART::dmaCallback();
  }
}

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

void USART1_IRQHandler(void) {
  HAL_UART_IRQHandler(&UART::huart1);
}

void DMA1_Stream5_IRQHandler(void) {
  HAL_DMA_IRQHandler(&UART::hdma_usart1_tx);
}

void EXTI15_10_IRQHandler(void) {
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if(GPIO_Pin == GPIO_PIN_13) {
    GPIO::buttonPressed = true;
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
  #if ENABLE_MICROSD
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
  #endif

  while (1) {
    printf("Task 1\n");
#if ENABLE_ALLOCATION_TRACKER
    Memory::printAllocations();
#endif
    Scheduler::yieldDelay(5000);
  }
}

// Display task
#if ENABLE_LCD
void task2(void) {
  // determine max possible full frame fps by drawing a black screen 10 times
  uint32_t start = HAL_GetTick();
  for (int i = 0; i < 10; i++) {
    LCD::fillRect(0, 0, LCD::WIDTH, LCD::HEIGHT, BLACK);
    LCD::update();
  }
  uint32_t end = HAL_GetTick();
  uint32_t dt = (end - start) / 10;
  // get spi clock for the lcd
  PLL2_ClocksTypeDef PLL2_Clocks;
  HAL_RCCEx_GetPLL2ClockFreq(&PLL2_Clocks);
  uint32_t spi_freq = PLL2_Clocks.PLL2_Q_Frequency;
  char string[32];

  // Variables for scrolling
  const uint8_t lineHeight = 12;
  const uint8_t screenHeight = LCD::HEIGHT;
  const uint8_t maxVisibleLines = screenHeight / lineHeight;  // 6 lines can fit on screen
  const uint8_t totalLines = 10;  // Increased for more info
  const uint8_t scrollSpeed = 1; // Pixels to scroll per update
  int16_t scrollPosition = 0;
  uint32_t lastScrollTime = HAL_GetTick();
  const uint32_t scrollInterval = 50; // Reduced interval for smoother scrolling
  const uint32_t pauseDuration = 2000; // 2 seconds pause at each end
  bool scrollingDown = true;
  uint32_t pauseStartTime = 0;
  bool isPaused = false;

  // Calculate maximum scroll position to align last line with bottom of screen
  const int16_t maxScrollPosition = (totalLines * lineHeight) - screenHeight;

  // Uptime tracking
  uint32_t startTime = HAL_GetTick();
  uint32_t lastUptimeUpdate = startTime;
  uint32_t uptimeSeconds = 0;

  uint32_t buttonPresses = 0;

  while (1) {
    // Update uptime
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastUptimeUpdate >= 1000) { // Update every second
      uptimeSeconds = (currentTime - startTime) / 1000;
      lastUptimeUpdate = currentTime;
    }

    // Get memory statistics
    Memory::MemoryRegion flash, ram, heap;
    Memory::getStats(flash, ram, heap);

    // Clear the screen
    LCD::fillRect(0, 0, LCD::WIDTH, LCD::HEIGHT, BLACK);

    if (GPIO::wasPressed()) {
      buttonPresses++;
    }

    if (buttonPresses % 2 == 0) {
      // Draw stats with scrolling offset
      // System Info
      sprintf(string, "CPU: %lu MHz, SPI: %lu MHz  ", HAL_RCC_GetSysClockFreq() / 1000000, spi_freq / 1000000 / 2);
      LCD::drawString(0, 0 - scrollPosition, 12, string);
      sprintf(string, "Frame: %lu ms (%lu Hz)  ", dt, 1000 / dt);
      LCD::drawString(0, lineHeight - scrollPosition, 12, string);
      
      // Uptime
      uint32_t hours = uptimeSeconds / 3600;
      uint32_t minutes = (uptimeSeconds % 3600) / 60;
      uint32_t seconds = uptimeSeconds % 60;
      sprintf(string, "Uptime: %02lu:%02lu:%02lu  ", hours, minutes, seconds);
      LCD::drawString(0, lineHeight * 2 - scrollPosition, 12, string);
      
      // Memory Info
      sprintf(string, "Flash: %lu / %lu KB  ", flash.used / 1024, flash.size / 1024);
      LCD::drawString(0, lineHeight * 3 - scrollPosition, 12, string);
      sprintf(string, "RAM: %lu / %lu KB  ", ram.used / 1024, ram.size / 1024);
      LCD::drawString(0, lineHeight * 4 - scrollPosition, 12, string);
      sprintf(string, "Heap: %lu / %lu KB  ", heap.used / 1024, heap.size / 1024);
      LCD::drawString(0, lineHeight * 5 - scrollPosition, 12, string);
      
      // Task Info
      sprintf(string, "Tasks: %d  ", Scheduler::taskCount);
      LCD::drawString(0, lineHeight * 6 - scrollPosition, 12, string);
      
      // Temperature
      float temp = ADC::getTemperature();
      sprintf(string, "Core Temp: %d.%d C %c   ", (uint32_t)temp, (uint32_t)(temp * 10) % 10, temp > 80 ? '!' : ' ');
      LCD::drawString(0, lineHeight * 7 - scrollPosition, 12, string);
      
      // Scroll position (debug)
      sprintf(string, "Scroll: %d  ", scrollPosition);
      LCD::drawString(0, lineHeight * 8 - scrollPosition, 12, string);

      // Button pressed
      sprintf(string, "Button: %d  ", buttonPresses);
      LCD::drawString(0, lineHeight * 9 - scrollPosition, 12, string);

      // Update the display
      LCD::update();

      // Handle scrolling and pausing
      if (isPaused) {
        // Check if pause duration has elapsed
        if (currentTime - pauseStartTime >= pauseDuration) {
          isPaused = false;
          lastScrollTime = currentTime;
        }
      } else {
        // Check if it's time to scroll
        if (currentTime - lastScrollTime >= scrollInterval) {
          if (scrollingDown) {
            scrollPosition += scrollSpeed;
            // Check if we've reached the bottom
            if (scrollPosition >= maxScrollPosition) {
              scrollPosition = maxScrollPosition;
              scrollingDown = false;
              isPaused = true;
              pauseStartTime = currentTime;
            }
          } else {
            scrollPosition -= scrollSpeed;
            // Check if we've reached the top
            if (scrollPosition <= 0) {
              scrollPosition = 0;
              scrollingDown = true;
              isPaused = true;
              pauseStartTime = currentTime;
            }
          }
          lastScrollTime = currentTime;
        }
      }
    } else {
      for (int i = 0; i < 10; i++) {
        ADC::read();
      }

      // draw voltage across screen as pixels where height is voltage
      uint8_t prev_voltage = 0;
      uint32_t avg = 0;
      for (int i = 0; i < LCD::WIDTH; i++) {
        uint32_t voltage = ADC::getVoltage();
        uint32_t height = voltage * LCD::HEIGHT / 3300;
        if (i == 0) {
          prev_voltage = height;
          continue; // dont draw, just update prev_voltage
        }
        // Invert Y coordinates by subtracting from LCD::HEIGHT
        LCD::drawLine(i - 1, LCD::HEIGHT - prev_voltage, i, LCD::HEIGHT - height, WHITE);
        prev_voltage = height;
        avg += height;
      }
      avg /= LCD::WIDTH;
      sprintf(string, "Avg: %d", avg);
      LCD::drawString(0, LCD::HEIGHT - 12, 12, string);

      // Update the display
      LCD::update();
    }
    Scheduler::yieldDelay(50);
  }
}
#endif

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
    printf("called task\n");
    Scheduler::yieldDelay(500);
  }
}

void exitingTask(void) {
  printf("Exiting task starting\n");
  Scheduler::yieldDelay(500);
  Scheduler::initTaskStack(calledTask, 256, "calledTask");
  Scheduler::yieldDelay(500);
  printf("Exiting task done\n");
}

int main(void) {
  if (HAL_Init() != HAL_OK) {
    ErrorHandler::handle(ErrorCode::HAL_INIT_FAILED, __FILE__, __LINE__);
  }
  Init_MPU();
  SCB_EnableICache();
  SCB_EnableDCache();
  SystemClock::init();
  SystemTick::init();
  GPIO::init();
  UART::init();
  Memory::init();
  SPI::init();
  Timer::init();
  
  #if ENABLE_MICROSD
  MicroSD::init();
  #endif

  #if ENABLE_LCD
  LCD::init();
  #endif

  // enable temperature sensor
  ADC3_COMMON->CCR |= ADC_CCR_TSEN;
  HAL_Delay(10); // Wait at least 10µs for sensor to stabilize

  ADC::init();
  ADC::calibrate();

  printf("Initializing tasks\n");

  Scheduler::initTaskStack(task1, 256, "task1");

  #if ENABLE_LCD
  Scheduler::initTaskStack(task2, 256, "task2");
  #endif

  Scheduler::initTaskStack(task3, 256, "task3");
  Scheduler::initTaskStack(exitingTask, 256, "exitingTask");

  Scheduler::start();

  // will not get here ideally
  ErrorHandler::handle(ErrorCode::UNKNOWN, __FILE__, __LINE__);
}