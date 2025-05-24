#pragma once

#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>

namespace UART {
  extern uint8_t tx_buffer[1024];
  extern uint8_t rx_buffer[1024];
  extern volatile size_t tx_head;
  extern volatile size_t tx_tail;
  extern volatile size_t rx_head;
  extern volatile size_t rx_tail;

  void init();
  void write(uint16_t size, uint8_t *data);
  size_t read(uint16_t size, uint8_t *data);
  size_t available();
} 