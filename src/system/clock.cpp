#include "clock.hpp"
#include "../error/handler.hpp"

void SystemClock::init() {
  configOscillator();
  configClock();
}

void SystemClock::exitRun0Mode() {
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) __asm__("NOP"); // Wait for VOSRDY flag
}

void SystemClock::configOscillator() {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) __asm__("NOP"); // Wait for VOSRDY flag

  // Configure PLL1 for system clock
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5; // 5 MHz
  RCC_OscInitStruct.PLL.PLLN = 140; // 700 MHz ( Unstable sonofabitch )
  RCC_OscInitStruct.PLL.PLLP = 1; // 700 MHz (SYSCLK)
  RCC_OscInitStruct.PLL.PLLQ = 4; // 175 MHz (D1PCLK1) for SDMMC
  RCC_OscInitStruct.PLL.PLLR = 2; // 350 MHz (D3PCLK1) for ADC
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    ErrorHandler::handle();
  }

  // Configure PLL2 for SPI4 and SDMMC for Main PLL
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI45 | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_ADC;
  PeriphClkInit.PLL2.PLL2M = 5; // 5 MHz
  PeriphClkInit.PLL2.PLL2N = 112; // 560 MHz
  PeriphClkInit.PLL2.PLL2P = 2; // unused
  PeriphClkInit.PLL2.PLL2Q = 8; // 70 MHz (PCLK2) for SPI4 (35 MHz with prescaler 2)
  PeriphClkInit.PLL2.PLL2R = 2; //
  PeriphClkInit.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
  PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInit.PLL2.PLL2FRACN = 0;
  PeriphClkInit.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
  PeriphClkInit.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP; 
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    ErrorHandler::handle();
  }
}

void SystemClock::configClock() {
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  // Increased flash latency for stable operation at 750 MHz
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    ErrorHandler::handle();
  }

  SystemCoreClockUpdate();
}

extern "C" {
  void ExitRun0Mode(void) {
    SystemClock::exitRun0Mode();
  }
} 
