# STM32H7 RTOS Project

A simple (and finally stable) RTOS implementation for STM32H7 microcontrollers. It's got the basics covered - task scheduling, GPIO stuff, UART for talking to your computer, LCD display support, microSD card support, and some timer magic.

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
- Comprehensive error handling system with:
  - Detailed error codes categorized by severity
  - Automatic error reporting over UART (if possible)
  - Critical error detection and hard fault triggering
  - Non-critical error handling with graceful recovery
  - Error categories:
    - System corruption errors (hard fault)
    - Recoverable system errors
    - Peripheral initialization errors
    - Storage errors
    - Hardware errors
    - Critical system failures (hard fault)
  - Visual error indication through LED patterns
  - File and line number tracking for debugging
  - Error message formatting with:
    - Error code in hexadecimal
    - Human-readable error description
    - Source file and line number
- GPIO controls for your LEDs and buttons:
  - LED toggle and state control
  - High-speed output configuration
  - Hardware flow control
  - Button press detection with interrupt support
- UART for sending messages to your computer:
  - High-speed 1.5 Mbaud rate
  - 8-bit data, 1 stop bit, no parity
  - Hardware flow control support
  - DMA-accelerated transmission with:
    - Zero-copy buffer management
    - Automatic memory allocation and cleanup
    - Interrupt-driven operation
    - Cooperative task yielding during transfers
    - Efficient memory handling with custom allocator
    - Automatic transfer completion detection
    - Error handling and recovery
  - Optimized for RTOS integration:
    - Non-blocking operation
    - Task-friendly yield mechanism
    - Memory-safe buffer handling
    - Automatic resource cleanup
    - Interrupt-safe state management
- Timer management for timing things
- System clock setup with optimized CPU frequency
- Error handling when things go wrong
- ADC3 temperature monitoring with:
  - 12-bit resolution
  - Internal temperature sensor support
  - Automatic calibration
  - Polling-based conversion
  - Temperature calculation in Celsius
  - Voltage measurement support
  - Dual-channel operation
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
  - Smooth scrolling text display with:
    - Configurable scroll speed and timing
    - Automatic pause at top and bottom positions
    - Real-time system information display
    - Memory usage monitoring
    - CPU and SPI clock speed display
    - Frame time and FPS calculation
    - Task count monitoring
    - Core temperature monitoring
    - Uptime tracking
    - Button press counter
- microSD card support with:
  - High-speed SPI communication
  - Card detection and initialization
  - Error handling and recovery
  - Block read/write operations
  - Card information retrieval
  - Interrupt-safe operations
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

- Task 1: microSD card task that:
  - Detects and initializes the SD card
  - Reads and displays card information
- Task 2: LCD display task that shows:
  - LCD ID and configuration
  - CPU frequency
  - SPI clock speed
  - Frame time and FPS
  - Current number of running tasks
  - Flash memory usage
  - RAM usage
  - Heap allocation status
  - Core temperature
  - Uptime in HH:MM:SS format
  - Button press counter
  - Voltage measurement visualization
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