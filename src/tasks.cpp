#include "main.h"

void task1(void) {
  while (1) {
    GPIO::toggleLed();
    TIM::delay_ms(1000);
  }
}

void handleCmd(const char *cmd, const char *args) {
  printf("cmd: %s, args: %s\n", cmd, args == NULL ? "" : args);
}

void task2(void) {
  uint8_t rx_buf[32];
  char cmd_buf[128];
  size_t cmd_pos = 0;
  size_t cursor = 0;

  UART::write(1, (uint8_t*)"\n");

  const char *prompt = "$ ";
  UART::write(2, (uint8_t*)prompt);

  enum { NORMAL, ESC, CSI } state = NORMAL;

  while (1) {
    size_t available = UART::available();
    if (available > 0) {
      size_t read = UART::read(available, rx_buf);
      for (size_t i = 0; i < read; ++i) {
        char c = rx_buf[i];

        // --- Escape sequence state machine ---
        if (state == NORMAL) {
          if (c == 0x1B) { state = ESC; continue; }
        } else if (state == ESC) {
          if (c == '[') { state = CSI; continue; }
          else { state = NORMAL; continue; }
        } else if (state == CSI) {
          if (c == 'D') { if (cursor > 0) { UART::write(3, (uint8_t*)"\x1B[D"); cursor--; } }
          else if (c == 'C') { if (cursor < cmd_pos) { UART::write(3, (uint8_t*)"\x1B[C"); cursor++; } }
          state = NORMAL;
          continue;
        }

        // --- Normal input handling ---
        if (c == '\r' || c == '\n') {
          UART::write(1, (uint8_t*)"\n");
          cmd_buf[cmd_pos] = '\0';

          // --- Split into cmd and args ---
          char *cmd = cmd_buf;
          // Skip leading spaces
          while (*cmd == ' ') cmd++;
          // Find first space (end of command)
          char *space = cmd;
          while (*space && *space != ' ') space++;
          char *args = NULL;
          if (*space) {
            *space = '\0';
            args = space + 1;
            // Skip leading spaces in args
            while (*args == ' ') args++;
            // If args is empty or only whitespace, set to NULL
            if (*args == '\0') args = NULL;
          }
          // If there were no spaces, args is NULL

          // If the command is empty, don't call handleCmd
          if (*cmd != '\0')
            handleCmd(cmd, args);
          // else: empty line, do nothing

          cmd_pos = 0;
          cursor = 0;
          UART::write(2, (uint8_t*)prompt);
        } else if (c == 0x7F || c == 0x08) { // Backspace
          if (cursor > 0) {
            for (size_t j = cursor - 1; j + 1 < cmd_pos; ++j)
              cmd_buf[j] = cmd_buf[j + 1];
            cmd_pos--;
            cursor--;
            UART::write(3, (uint8_t*)"\b \b");
            UART::write(cmd_pos - cursor, (uint8_t*)&cmd_buf[cursor]);
            UART::write(1, (uint8_t*)" ");
            for (size_t j = cursor; j < cmd_pos + 1; ++j)
              UART::write(3, (uint8_t*)"\x1B[D");
          }
        } else if (cmd_pos < sizeof(cmd_buf) - 1 && c >= 0x20 && c <= 0x7E) {
          for (size_t j = cmd_pos; j > cursor; --j)
            cmd_buf[j] = cmd_buf[j - 1];
          cmd_buf[cursor] = c;
          cmd_pos++;
          cursor++;
          UART::write(1, (uint8_t*)&c);
          UART::write(cmd_pos - cursor, (uint8_t*)&cmd_buf[cursor]);
          for (size_t j = cursor; j < cmd_pos; ++j)
            UART::write(3, (uint8_t*)"\x1B[D");
        }
      }
    }
  }
}
