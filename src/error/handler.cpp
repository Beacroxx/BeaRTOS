#include "handler.hpp"
#include "../peripherals/gpio.hpp"

// Error handler
void ErrorHandler::handle() {
  while (1) {
    GPIO::toggleLed();
    HAL_Delay(50); 
  }
}

// Hard fault handler
void ErrorHandler::hardFault() {
  while (1) {
    GPIO::toggleLed();
    HAL_Delay(20); // Faster delay for hardfault
  }
} 