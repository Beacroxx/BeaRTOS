#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_adc.h"

// Constants for the NTC thermistor
#define V30       620 // mV (Datasheet P278)
#define AVG_SLOPE 2   // mV/C

namespace ADC {
void init();
void calibrate();
void read();
float getTemperature();
uint32_t getVoltage();

extern ADC_HandleTypeDef hadc3;
extern ADC_ChannelConfTypeDef sConfig1;
extern uint32_t values[2];
} // namespace ADC
