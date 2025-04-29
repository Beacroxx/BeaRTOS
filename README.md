# STM32H7 RTOS Project

A simple RTOS implementation for STM32H7 microcontrollers. It's got the basics covered - task scheduling, GPIO stuff, UART for talking to your computer, and some timer magic.

## What's Inside

- A basic RTOS that can run multiple tasks
- GPIO controls for your LEDs and buttons
- UART for sending messages to your computer
- Timer management for timing things
- System clock setup
- Error handling when things go wrong
- A simple LED blinky example

## What You'll Need

- PlatformIO
- A WeAct STM32H7 board
- ARM GCC toolchain (comes with PlatformIO)
- The [Board definition](stm32h723weact.json) in `/home/$USER/.platformio/platforms/ststm32/`

## How It's Organized

```
src/
├── error/       # For when things go wrong
├── peripherals/ # The fun stuff (GPIO, UART, Timer)
└── system/      # The brain of the operation
```

## Getting Started

1. Get the code (clone the repo)
2. Open it up in PlatformIO
3. Hit the build button

## What It Does

There are three example tasks running:

- Task 1: Says "Task 1" every 3 seconds
- Task 2: Says "Task 2" every 2 seconds
- Task 3: Says "Task 3" and blinks an LED every second

## License

This project is under the MIT License - check out [LICENSE.md](LICENSE.md) for the details. 