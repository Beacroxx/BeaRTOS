#include "main.h"

namespace UART {
uint8_t tx_buffer[1024];
uint8_t rx_buffer[1024];

volatile size_t tx_head = 0;
volatile size_t tx_tail = 0;
volatile size_t rx_head = 0;
volatile size_t rx_tail = 0;

void init() {
  rcc_periph_clock_enable(RCC_USART1);
  rcc_periph_clock_enable(RCC_DMA1);

  usart_set_baudrate(USART1, 1500000);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART1, USART_MODE_TX_RX);

  usart_enable_rx_interrupt(USART1);

  nvic_enable_irq(NVIC_USART1_IRQ);

  usart_enable(USART1);
}

void write(uint16_t size, uint8_t *data) {
  for (size_t i = 0; i < size; i++) {
    size_t next_head = (tx_head + 1) % sizeof(tx_buffer);
    while (next_head == tx_tail) {
      yield();
    }
    tx_buffer[tx_head] = data[i];
    tx_head = next_head;
  }
  cm_disable_interrupts();
  if (usart_get_flag(USART1, USART_ISR_TXE)) {
    if (tx_head != tx_tail) {
      usart_send(USART1, tx_buffer[tx_tail]);
      tx_tail = (tx_tail + 1) % sizeof(tx_buffer);
    }
  }
  usart_enable_tx_interrupt(USART1);
  cm_enable_interrupts();
}

size_t read(uint16_t size, uint8_t *data) {
  size_t count = 0;
  while (rx_tail != rx_head && count < size) {
    data[count++] = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % sizeof(rx_buffer);
  }
	data[count] = '\0';
  return count;
}

size_t available() {
  return (rx_head - rx_tail) % sizeof(rx_buffer);
}
} // namespace UART 

extern "C" {
// overwrite _write to redirect to UART::write
int _write(int file, char *ptr, int len) {
  (void)file; // unused
  UART::write(len, (uint8_t *)ptr);
  return len;
}

// USART1 ISR
void usart1_isr(void) {
  if (usart_get_flag(USART1, USART_ISR_RXNE)) {
    uint8_t data = usart_recv(USART1);
    if ((UART::rx_head + 1) % sizeof(UART::rx_buffer) != UART::rx_tail) {
      UART::rx_buffer[UART::rx_head] = data;
      UART::rx_head = (UART::rx_head + 1) % sizeof(UART::rx_buffer);
    }
  } else if (usart_get_flag(USART1, USART_ISR_TXE)) {
    if (UART::tx_head == UART::tx_tail) {
      usart_disable_tx_interrupt(USART1);
    } else {
      usart_send(USART1, UART::tx_buffer[UART::tx_tail]);
      UART::tx_tail = (UART::tx_tail + 1) % sizeof(UART::tx_buffer);
    }
  }
}
}