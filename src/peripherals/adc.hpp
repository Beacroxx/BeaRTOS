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
	static uint32_t read();
	static float getTemperature();
	
private:
	static ADC_HandleTypeDef hadc3;
	static ADC_ChannelConfTypeDef sConfig1;
};
