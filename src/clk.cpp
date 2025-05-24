#include "main.h"

namespace CLK {
uint32_t sysclk_frequency = 0;

void init() {
  // clang-format off
  struct rcc_pll_config my_pll_config = {
    .sysclock_source = RCC_PLL, // Use PLL1 as SYSCLK source
    .pll_source = RCC_PLLCKSELR_PLLSRC_HSE, // PLL source is HSE
    .hse_frequency = 25000000, // 25 MHz external crystal
    .pll1 = {
      .divm = 5,   // PLL1 DIVM1 (25 MHz / 5 = 5 MHz)
      .divn = 110, // PLL1 DIVN1 (5 MHz * 110 = 550 MHz)
      .divp = 1,   // PLL1 DIVP1 (PLLP output, SYSCLK) (550 MHz / 1 = 550 MHz)
      .divq = 2,   // PLL1 DIVQ1 (for peripherals, e.g. USB) (550 MHz / 2 = 275 MHz)
      .divr = 2,   // PLL1 DIVR1 (for peripherals) (550 MHz / 2 = 275 MHz)
    },
    .pll2 = {
      .divm = 1, // Not used, set to safe values
      .divn = 8,
      .divp = 2,
      .divq = 2,
      .divr = 2,
    },
    .pll3 = {
      .divm = 1, // Not used, set to safe values
      .divn = 8,
      .divp = 2,
      .divq = 2,
      .divr = 2,
    },
    .core_pre = RCC_D1CFGR_D1CPRE_BYP, // Core prescaler: no division
    .hpre = RCC_D1CFGR_D1HPRE_DIV2, // HCLK = SYSCLK / 2 = 275 MHz (max for H7)
    .ppre1 = RCC_D2CFGR_D2PPRE_DIV2, // APB1 = HCLK / 2 = 137.5 MHz
    .ppre2 = RCC_D2CFGR_D2PPRE_DIV2, // APB2 = HCLK / 2 = 137.5 MHz
    .ppre3 = RCC_D1CFGR_D1PPRE_DIV2, // APB3 = HCLK / 2 = 137.5 MHz
    .ppre4 = RCC_D3CFGR_D3PPRE_DIV2, // APB4 = HCLK / 2 = 137.5 MHz
    .flash_waitstates = 7, // 7 WS for 2.7-3.6V at 550 MHz (see RM0433 Table 19)
    .voltage_scale = PWR_VOS_SCALE_1, // Voltage Scale 1 for >400 MHz
    .power_mode = PWR_SYS_SMPS_DIRECT, // Use SMPS
    .smps_level = 0, // Not used for LDO
  };
  // clang-format on
  sysclk_frequency = 550000000; // 550 MHz

  rcc_clock_setup_pll(&my_pll_config);
}
} // namespace CLK 