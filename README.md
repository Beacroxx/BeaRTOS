# STM32H7 RTOS Project

A simple (but unstable) RTOS implementation for STM32H7 microcontrollers. It's got the basics covered - task scheduling, GPIO stuff, UART for talking to your computer, LCD display support, microSD card support, and some timer magic.

## What's Inside

- A basic RTOS that can run multiple tasks with dynamic memory allocation
- Automatic task memory reclamation for terminated tasks
- Memory and heap monitoring with:
  - Flash memory usage tracking
  - RAM usage tracking
  - Heap allocation tracking
  - Memory region statistics
  - Automatic memory leak detection
  - Memory usage display on LCD
- Memory Protection Unit (MPU) configuration for:
  - AXI SRAM region protection
  - Write-through caching
  - Cacheable memory regions
  - Non-shareable memory access
  - Full access permissions
- GPIO controls for your LEDs and buttons:
  - LED toggle and state control
  - High-speed output configuration
  - Hardware flow control
- UART for sending messages to your computer
- Timer management for timing things
- System clock setup with optimized CPU frequency
- Error handling when things go wrong
- ST7735 LCD display support with:
  - Text rendering with two font sizes:
    - 12x6 pixel font for compact display
    - 16x8 pixel font for better readability
  - Full ASCII character set support
  - Graphics primitives (lines, rectangles, pixels)
  - RGB565 color support
  - Display window control
  - Brightness control
  - DMA-accelerated SPI communication for improved performance
- microSD card support with:
  - High-speed SPI communication
  - Card detection and initialization
  - Error handling and recovery
- A simple LED blinky example
- Cooperative yield delay for efficient task waiting
- SPI communication with:
  - DMA-accelerated transfers
  - Interrupt-driven operation
  - Configurable baud rates
  - Hardware flow control
  - CRC calculation support
  - FIFO threshold configuration

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
  - Flash memory usage
  - RAM usage
  - Heap allocation status
- Task 3: Prints "Task 3" and toggles an LED every second
- Task 4: A task that demonstrates task creation and termination:
  - Creates a new task that displays system diagnostics
  - Exits after task creation

The scheduler automatically manages task memory:
- Tasks are dynamically allocated when created
- Memory is automatically reclaimed when tasks terminate
- The system reuses memory from terminated tasks for new tasks
- Each task gets a unique name (randomly generated if not specified)
- Memory usage is tracked and displayed in real-time
- Heap allocations are monitored for potential memory leaks
- Memory regions (Flash, RAM, Heap) are tracked and reported
- Active task count is automatically tracked and displayed

Note: The system has been updated to support printf functionality from both task and non-task contexts, making debugging and logging more flexible throughout the application.

The memory monitoring system provides:
- Real-time tracking of Flash memory usage
- RAM usage monitoring including data and BSS sections
- Heap allocation tracking with used and free space
- Memory region statistics for system optimization
- Automatic memory leak detection through allocation tracking
- Visual feedback through the LCD display

The system includes comprehensive idle time monitoring:
- Tracks CPU idle time percentage in real-time
- Displays idle time statistics on the LCD

The RTOS includes a yield delay mechanism that allows tasks to wait efficiently:
- Tasks can voluntarily yield their execution time while waiting
- Other tasks continue running during the delay period
- The scheduler remains preemptive, but tasks can cooperate for better resource utilization
- Yield delays are implemented using the system tick timer for precise timing

The LCD display uses DMA for efficient data transfer:
- SPI4 is configured with DMA for high-speed data transfer
- DMA transfers are handled automatically in the background
- Tasks can yield while waiting for DMA transfers to complete
- Interrupt handlers manage DMA transfer completion

## License

This project is under the MIT License - check out [LICENSE.md](LICENSE.md) for the details. 