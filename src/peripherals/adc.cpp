#include "adc.hpp"
#include "error/handler.hpp"

#include <stdio.h>

ADC_HandleTypeDef ADC::hadc3;
ADC_ChannelConfTypeDef ADC::sConfig1;
uint32_t ADC::values[2];

void ADC::init() {
  __HAL_RCC_ADC3_CLK_ENABLE();

  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8; // 350 MHz / 8 = 43.75 MHz
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.DataAlign = ADC3_DATAALIGN_RIGHT;
  hadc3.Init.ScanConvMode = ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = ENABLE; // enable continuous conversion
  hadc3.Init.NbrOfConversion = 2;
  hadc3.Init.DiscontinuousConvMode = ENABLE;
  hadc3.Init.NbrOfDiscConversion = 1;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.SamplingMode = ADC_SAMPLING_MODE_NORMAL;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;

  if (HAL_ADC_Init(&hadc3) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::ADC_INIT_FAILED, __FILE__, __LINE__);
  }

  sConfig1.Channel = ADC_CHANNEL_TEMPSENSOR; // temperature sensor
  sConfig1.Rank = ADC_REGULAR_RANK_1;
  sConfig1.SamplingTime = ADC3_SAMPLETIME_247CYCLES_5; // 247.5 cycles
  sConfig1.SingleDiff = ADC_SINGLE_ENDED;
  sConfig1.OffsetNumber = ADC_OFFSET_NONE;
  sConfig1.Offset = 0;
  sConfig1.OffsetSign = ADC3_OFFSET_SIGN_NEGATIVE;

  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig1) != HAL_OK) {
    ErrorHandler::handle(ErrorCode::ADC_CHANNEL_CONFIG_FAILED, __FILE__, __LINE__);
  }

	sConfig1.Channel = ADC_CHANNEL_11; // PC1
  sConfig1.Rank = ADC_REGULAR_RANK_2;
  sConfig1.SamplingTime = ADC3_SAMPLETIME_24CYCLES_5; // Faster sampling for scope
	if (HAL_ADC_ConfigChannel(&hadc3, &sConfig1) != HAL_OK) {
		ErrorHandler::handle(ErrorCode::ADC_CHANNEL_CONFIG_FAILED, __FILE__, __LINE__);
	}
}

void ADC::calibrate() {
	if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
		ErrorHandler::handle(ErrorCode::ADC_CALIBRATION_FAILED, __FILE__, __LINE__);
	}
}

void ADC::read() {
  for (uint32_t i = 0; i < 2; i++) {
    if (HAL_ADC_Start(&hadc3) != HAL_OK) {
      ErrorHandler::handle(ErrorCode::ADC_READ_FAILED, __FILE__, __LINE__);
    }

    if (HAL_ADC_PollForConversion(&hadc3, 1000) != HAL_OK) {
      ErrorHandler::handle(ErrorCode::ADC_READ_FAILED, __FILE__, __LINE__);
    }

    values[i] = HAL_ADC_GetValue(&hadc3);
  }
}

float ADC::getTemperature() {
  read();
	return (((float)values[0] * 3300) / 4096 - V30) / AVG_SLOPE + 30;
}

uint32_t ADC::getVoltage() {
  read();
	return ((float)values[1] * 3300) / 4096;
}