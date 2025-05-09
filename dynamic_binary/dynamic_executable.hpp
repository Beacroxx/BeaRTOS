#pragma once

// Standard C++ headers
#include <cstdint>
#include <cstdarg>
#include <cstdio>

// Core STM32H7xx definitions
#include "stm32h7xx.h"

// STM32 HAL headers
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

// System headers
#include "../src/system/scheduler.hpp"
#include "../src/system/memory.hpp"
#include "../src/system/clock.hpp"
#include "../src/system/systick.hpp"
#include "../src/peripherals/uart.hpp"
#include "../src/peripherals/timer.hpp"
#include "../src/peripherals/spi.hpp"
#include "../src/peripherals/microsd.hpp"
#include "../src/peripherals/lcd.hpp"
#include "../src/peripherals/gpio.hpp"
#include "../src/peripherals/font.hpp"
#include "../src/peripherals/adc.hpp"
#include "../src/error/handler.hpp"
#include "../src/error/codes.hpp"
#include "../src/middleware/FatFs/fatfs.hpp"
#include "../src/middleware/FatFs/ff.h"

// C++ support functions
extern "C" {
  __attribute__((section(".data"))) void* __dso_handle = nullptr;
  __attribute__((section(".text"))) void __cxa_atexit() {}
  __attribute__((section(".text"))) void __cxa_finalize() {}
  __attribute__((section(".text"))) void __cxa_guard_acquire() {}
  __attribute__((section(".text"))) void __cxa_guard_release() {}
  __attribute__((section(".text"))) void __cxa_pure_virtual() {}
  __attribute__((section(".text"))) void __aeabi_unwind_cpp_pr0() {}
  __attribute__((section(".text"))) void __aeabi_unwind_cpp_pr1() {}
  __attribute__((section(".text"))) void __aeabi_unwind_cpp_pr2() {}
} 

// This is the entry point that will be called when the executable is loaded
extern "C" __attribute__((section(".text.entry"))) int main();