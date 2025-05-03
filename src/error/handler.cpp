#include "handler.hpp"
#include "../peripherals/gpio.hpp"
#include "../peripherals/uart.hpp"
#include "../system/scheduler.hpp"
#include <cstdio>
#include <cstring>

// Error handler
void ErrorHandler::handle(ErrorCode code, const char* file, int line) {
  reportError(code, file, line);
  
  if (isCriticalError(code)) {
    hardFault(code, file, line);
  }

  // return silently if not critical
  return;
}

// Hard fault handler
void ErrorHandler::hardFault(ErrorCode code, const char* file, int line) {
  reportError(code, file, line);
  
  while (1) {
    GPIO::toggleLed();
    HAL_Delay(20); // Faster delay for hardfault
  }
}

void ErrorHandler::reportError(ErrorCode code, const char* file, int line) {
  printErrorOverUart(code, file, line);
}

#if ENABLE_ERROR_STRINGS
const char* ErrorHandler::getErrorString(ErrorCode code) {
  switch (code) {
    // System errors that should trigger hard fault
    case ErrorCode::NONE: return "No error";
    case ErrorCode::UNKNOWN: return "Unknown error";
    case ErrorCode::HARD_FAULT: return "Hard fault occurred";
    case ErrorCode::MEMORY_CORRUPTION: return "Memory corruption detected";
    case ErrorCode::STACK_CORRUPTION: return "Stack corruption detected";
    case ErrorCode::TASK_STATE_CORRUPTION: return "Task state corruption detected";
    case ErrorCode::TASK_STACK_CORRUPTION: return "Task stack corruption detected";
    case ErrorCode::SCHEDULER_CORRUPTION: return "Scheduler corruption detected";
    case ErrorCode::INTERRUPT_CORRUPTION: return "Interrupt corruption detected";
    
    // Recoverable system errors
    case ErrorCode::MEMORY_ALLOCATION_FAILED: return "Memory allocation failed";
    case ErrorCode::STACK_OVERFLOW: return "Stack overflow detected";
    case ErrorCode::INVALID_TASK_STATE: return "Invalid task state";
    case ErrorCode::TASK_SCHEDULING_ERROR: return "Task scheduling error";
    case ErrorCode::SYSTEM_CLOCK_ERROR: return "System clock error";
    case ErrorCode::POWER_MANAGEMENT_ERROR: return "Power management error";
    
    // Peripheral errors
    case ErrorCode::ADC_INIT_FAILED: return "ADC initialization failed";
    case ErrorCode::ADC_CALIBRATION_FAILED: return "ADC calibration failed";
    case ErrorCode::ADC_READ_FAILED: return "ADC read failed";
    case ErrorCode::ADC_CHANNEL_CONFIG_FAILED: return "ADC channel config failed";
    
    case ErrorCode::UART_INIT_FAILED: return "UART initialization failed";
    case ErrorCode::UART_TRANSMIT_FAILED: return "UART transmit failed";
    case ErrorCode::UART_RECEIVE_FAILED: return "UART receive failed";
    
    case ErrorCode::SPI_INIT_FAILED: return "SPI initialization failed";
    case ErrorCode::SPI_TRANSFER_FAILED: return "SPI transfer failed";
    case ErrorCode::SPI_DMA_INIT_FAILED: return "SPI DMA initialization failed";
    case ErrorCode::SPI_DMA_TRANSFER_FAILED: return "SPI DMA transfer failed";
    
    case ErrorCode::TIMER_INIT_FAILED: return "Timer initialization failed";
    case ErrorCode::TIMER_CONFIG_FAILED: return "Timer configuration failed";
    case ErrorCode::TIMER_PWM_CONFIG_FAILED: return "Timer PWM configuration failed";
    
    case ErrorCode::GPIO_INIT_FAILED: return "GPIO initialization failed";
    case ErrorCode::GPIO_CONFIG_FAILED: return "GPIO configuration failed";
    
    // Storage errors
    case ErrorCode::SD_CARD_INIT_FAILED: return "SD card initialization failed";
    case ErrorCode::SD_CARD_NOT_PRESENT: return "SD card not present";
    case ErrorCode::SD_CARD_READ_FAILED: return "SD card read failed";
    case ErrorCode::SD_CARD_WRITE_FAILED: return "SD card write failed";
    case ErrorCode::SD_CARD_BUS_WIDTH_ERROR: return "SD card bus width error";
    
    // Hardware errors
    case ErrorCode::DMA_INIT_FAILED: return "DMA initialization failed";
    case ErrorCode::DMA_TRANSFER_FAILED: return "DMA transfer failed";
    case ErrorCode::DMA_CONFIG_FAILED: return "DMA configuration failed";
    
    // Critical errors
    case ErrorCode::CRITICAL_MEMORY_CORRUPTION: return "Critical memory corruption";
    case ErrorCode::CRITICAL_STACK_CORRUPTION: return "Critical stack corruption";
    case ErrorCode::CRITICAL_HARDWARE_FAILURE: return "Critical hardware failure";
    case ErrorCode::CRITICAL_SYSTEM_HANG: return "Critical system hang";
    case ErrorCode::CRITICAL_INTERRUPT_FAILURE: return "Critical interrupt failure";
    case ErrorCode::CRITICAL_TASK_FAILURE: return "Critical task failure";
    case ErrorCode::CRITICAL_SCHEDULER_FAILURE: return "Critical scheduler failure";
    case ErrorCode::HAL_INIT_FAILED: return "HAL initialization failed";

    default: return "Unknown error code";
  }
}
#endif

bool ErrorHandler::isCriticalError(ErrorCode code) {
  uint32_t codeValue = static_cast<uint32_t>(code);
  // Critical errors are those in ranges 0x0000-0x0FFF and 0xF000-0xFFFF
  return (codeValue < 0x1000) || (codeValue >= 0xF000);
}

void ErrorHandler::printErrorOverUart(ErrorCode code, const char* file, int line) {
  // Try to print error message over UART
  // If UART is not working, this will fail silently
  char buffer[128];
  #if ENABLE_ERROR_STRINGS
  snprintf(buffer, sizeof(buffer), "Error: 0x%04X - %s (File: %s, Line: %d)\r\n", 
           static_cast<uint32_t>(code), getErrorString(code), file, line);
  #else
  snprintf(buffer, sizeof(buffer), "Error: 0x%04X (File: %s, Line: %d)\r\n", 
           static_cast<uint32_t>(code), file, line);
  #endif
  UART::write(1, buffer, strlen(buffer));
} 