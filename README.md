# STM32H7 RTOS Project

A lightweight RTOS implementation using libopencm3 for STM32H7 microcontrollers featuring task management and peripheral support.

## Core Features

### RTOS Core
- Preemptive task scheduling
- Task state management
- Task Control Block (TCB) based task management
- Task naming support
- Cooperative yield mechanism

### Memory Management
- Stack-based task memory allocation
- Task stack management
- Memory-efficient design

### Peripheral Support

#### UART Communication
- High-speed 1.5 Mbaud operation
- Circular buffer implementation for TX/RX
- Interrupt-driven operation
- Buffer size: 1024 bytes for both TX and RX
- Basic read/write operations

#### GPIO and Timer
- LED control
- Timer management system
- System clock configuration

## Project Structure
```
src/
├── main.h           # Main header with common includes and declarations
├── main.cpp         # Main application entry point and ISRs
├── uart.h/cpp       # UART communication module
├── gpio.h/cpp       # GPIO control module
├── clk.h/cpp        # Clock configuration module
├── tim.h/cpp        # Timer and delay functions
├── sched.h/cpp      # Task scheduler implementation
├── sched_asm.s      # Assembly routines for task switching
└── tasks.cpp        # Application tasks
```

## Requirements
- PlatformIO
- WeAct STM32H7 board
- ARM GCC toolchain (included with PlatformIO)
- Board definition file: `stm32h723weact.json` in `/home/$USER/.platformio/platforms/ststm32/`

## Getting Started
1. Clone the repository
2. Open in PlatformIO
3. Move/Copy board definition into PlatformIO
4. Build and flash

## License
MIT License - See [LICENSE.md](LICENSE.md) for details. 