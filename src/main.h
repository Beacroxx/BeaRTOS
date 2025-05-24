#pragma once

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart.h"
#include "gpio.h"
#include "clk.h"
#include "tim.h"
#include "sched.h"

inline void yield() {
  SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

void task1(void);
void task2(void); 