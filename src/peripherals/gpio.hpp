#pragma once

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOE
#define LCD_WR_RS_Pin GPIO_PIN_13
#define LCD_WR_RS_GPIO_Port GPIOE

#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOE

#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOC

class GPIO {
public:
  static void init();
  static void toggleLed();
  static void setLed(bool state);
  static bool wasPressed();
  static bool buttonPressed;

}; 