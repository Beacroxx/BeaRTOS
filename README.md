# STM32H7 RTOS Project

A (maybe) robust and stable RTOS implementation for STM32H7 microcontrollers featuring comprehensive task management, memory protection, and peripheral support.

## Core Features

### RTOS Core
- Preemptive task scheduling with dynamic memory allocation
- Automatic task memory reclamation
- Cooperative yield delay mechanism
- Task creation and termination management
- Unique task naming system

### Memory Management
- Memory Protection Unit (MPU) configuration:
  - AXI SRAM region protection
  - Write-through caching
  - Cacheable memory regions
  - Non-shareable memory access
  - Full access permissions
- Comprehensive memory monitoring:
  - Flash memory usage tracking
  - RAM usage tracking (data and BSS sections)
  - Heap allocation monitoring
  - Memory region statistics
  - Automatic memory leak detection
  - Real-time memory usage display
- Advanced Allocation Tracking (optional):
  - File and line number tracking for each allocation
  - Active allocation listing with source locations
  - Memory leak detection with source context
  - Allocation size tracking
  - Real-time allocation statistics

### Feature Configuration
The project supports selective feature enablement through build flags in `platformio.ini`:
- `ENABLE_ERROR_STRINGS`: Enable detailed error messages
- `ENABLE_ALLOCATION_TRACKER`: Enable memory allocation tracking
- `ENABLE_MICROSD`: Enable microSD card support
- `ENABLE_LCD`: Enable LCD display support

Example configuration:
```ini
[env:stm32h723weact]
platform = ststm32
board = stm32h723weact
framework = stm32cube
monitor_speed = 1500000
build_flags = 
  -D ENABLE_ERROR_STRINGS = 1
  -D ENABLE_ALLOCATION_TRACKER = 1
  -D ENABLE_MICROSD = 0
  -D ENABLE_LCD = 1
```

### Error Handling System
- Multi-level error categorization:
  - System corruption errors (hard fault)
  - Recoverable system errors
  - Peripheral initialization errors
  - Storage errors
  - Hardware errors
  - Critical system failures
- Error reporting mechanisms:
  - UART-based error logging
  - LED pattern indication
  - File and line number tracking
  - Formatted error messages with:
    - Hexadecimal error codes
    - Human-readable descriptions
    - Source location information

### Peripheral Support

#### UART Communication
- High-speed 1.5 Mbaud operation
- 8-bit data, 1 stop bit, no parity
- Hardware flow control
- DMA-accelerated transmission:
  - Zero-copy buffer management
  - Automatic memory allocation
  - Interrupt-driven operation
  - Task-friendly yield mechanism
  - Efficient memory handling
  - Automatic transfer completion
  - Error recovery

#### LCD Display (ST7735)
- Text rendering with dual font sizes:
  - 12x6 pixel compact font
  - 16x8 pixel standard font
- Full ASCII character support
- Graphics primitives
- RGB565 color support
- DMA-accelerated SPI communication
- Real-time system information display:
  - CPU/SPI clock speeds
  - Frame time and FPS
  - Task count
  - Memory usage
  - Core temperature
  - Uptime
  - Button press counter
  - Voltage visualization

#### microSD Card Support
- High-speed SPI communication
- Card detection and initialization
- Error handling and recovery
- Block read/write operations
- Card information retrieval
- Interrupt-safe operations

#### GPIO and Timer
- LED control with high-speed output
- Button press detection with interrupts
- Hardware flow control
- Timer management system
- System clock optimization

#### ADC3 Temperature Monitoring
- 12-bit resolution
- Internal temperature sensor
- Automatic calibration
- Polling-based conversion
- Celsius temperature calculation
- Voltage measurement
- Dual-channel operation

#### SPI Communication
- DMA-accelerated transfers
- Interrupt-driven operation
- Configurable baud rates
- Hardware flow control
- CRC calculation
- FIFO threshold configuration

## Project Structure
```
src/
├── error/       # Error handling and reporting
├── peripherals/ # Hardware peripheral drivers
└── system/      # RTOS core and system management
```

## Requirements
- PlatformIO
- WeAct STM32H7 board with LCD
- ARM GCC toolchain (included with PlatformIO)
- Board definition file: `stm32h723weact.json` in `/home/$USER/.platformio/platforms/ststm32/`

## Example Tasks
1. microSD Card Task:
   - Card detection and initialization
   - Information display
2. LCD Display Task:
   - System diagnostics display
   - Real-time monitoring
3. LED Blinky Task:
   - LED toggling demonstration
4. Task Management Demo:
   - Dynamic task creation
   - System diagnostics display

## Getting Started
1. Clone the repository
2. Open in PlatformIO
3. Move/Copy board definition into PlatformIO
4. Build and flash

## License
MIT License - See [LICENSE.md](LICENSE.md) for details. 