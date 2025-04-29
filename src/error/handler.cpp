#include "handler.hpp"
#include "../peripherals/gpio.hpp"

void ErrorHandler::handle() {
  while (1) {
    GPIO::toggleLed();
    HAL_Delay(50); 
  }
}

void ErrorHandler::hardFault() {
  while (1) {
    GPIO::toggleLed();
    HAL_Delay(20); // Faster delay for hardfault
  }
} 