#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_adc.h"

// Constants for the NTC thermistor
#define V30 620 // mV (Datasheet P278)
#define AVG_SLOPE 2 // mV/C

class ADC {
public:
	static void init();
	static void calibrate();
	static void read();
	static float getTemperature();
	static uint32_t getVoltage();
	
private:
	static ADC_HandleTypeDef hadc3;
	static ADC_ChannelConfTypeDef sConfig1;
	static uint32_t values[2];
};
