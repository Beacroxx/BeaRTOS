# STM32H7 RTOS Project

A simple RTOS implementation for STM32H7 microcontrollers. It's got the basics covered - task scheduling, GPIO stuff, UART for talking to your computer, LCD display support, and some timer magic.

## What's Inside

- A basic RTOS that can run multiple tasks with dynamic memory allocation
- Automatic task memory reclamation for terminated tasks
- GPIO controls for your LEDs and buttons
- UART for sending messages to your computer
- Timer management for timing things
- System clock setup with optimized CPU frequency
- Error handling when things go wrong
- ST7735 LCD display support with:
  - Text rendering (12x6 and 16x8 fonts)
  - Graphics primitives (lines, rectangles, pixels)
  - RGB565 color support
  - Display window control
  - Brightness control
- A simple LED blinky example
- Cooperative yield delay for efficient task waiting

## What You'll Need

- PlatformIO
- A WeAct STM32H7 board with LCD
- ARM GCC toolchain (comes with PlatformIO)
- The [Board definition](stm32h723weact.json) in `/home/$USER/.platformio/platforms/ststm32/`

## How It's Organized

```
src/
├── error/       # For when things go wrong
├── peripherals/ # The fun stuff (GPIO, UART, Timer, LCD, SPI)
└── system/      # The brain of the operation
```

## Getting Started

1. Get the code (clone the repo)
2. Open it up in PlatformIO
3. Hit the build button

## What It Does

There are four example tasks running:

- Task 1: Prints "Task 1" every second
- Task 2: LCD display task that shows:
  - LCD ID and configuration
  - CPU frequency
  - SPI clock speed
  - Frame time and FPS
  - Current number of running tasks
- Task 3: Prints "Task 3" and toggles an LED every second
- Task 4: A task that demonstrates task creation and termination:
  - Creates a new task that displays system diagnostics
  - Exits after task creation

The scheduler automatically manages task memory:
- Tasks are dynamically allocated when created
- Memory is automatically reclaimed when tasks terminate
- The system reuses memory from terminated tasks for new tasks
- Each task gets a unique name (randomly generated if not specified)

The RTOS includes a yield delay mechanism that allows tasks to wait efficiently:
- Tasks can voluntarily yield their execution time while waiting
- Other tasks continue running during the delay period
- The scheduler remains preemptive, but tasks can cooperate for better resource utilization
- Yield delays are implemented using the system tick timer for precise timing

## License

This project is under the MIT License - check out [LICENSE.md](LICENSE.md) for the details. 