# STM32H7 RTOS Project

A (potentially) robust and stable RTOS implementation for STM32H7 microcontrollers featuring comprehensive task management, memory protection, and peripheral support.

## Core Features

### RTOS Core
- Preemptive task scheduling with dynamic memory allocation
- Automatic task memory reclamation
- Cooperative yield delay mechanism
- Task creation and termination management
- Unique task naming system

### Dynamic Binary Loading
- Runtime loading and execution of compiled binaries
- Support for dynamic task creation from external binaries
- Integration with FatFS for binary storage and loading
- Symbol table management for dynamic linking
- Important considerations:
  - Symbols not used in the main program are optimized out and unavailable
  - Firmware recompilation requires recompilation of all dynamic binaries
  - Dynamic binaries must be compiled with matching symbol tables
  - Limited to functions and symbols explicitly included in the main firmware
  - Supports basic C++ features and everything in the RTOS core that is not optimized out

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

#### FatFS Middleware
- Full FatFS implementation (R0.15)
- Multiple volume support
- Long filename support (LFN)
- Dynamic memory allocation
- File system operations:
  - File open/close
  - Read/write operations
  - Directory operations
  - File information retrieval
- Error handling and recovery
- Task-safe operations
- Performance optimizations:
  - Buffer management
  - Cache utilization
  - Zero-copy operations
- Integration with microSD card driver
- File system monitoring and statistics

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
├── error/                 # Error handling and reporting
├── peripherals/           # Hardware peripheral drivers
├── system/                # RTOS core and system management
└── middleware/            # Middleware
    ├── FatFs/             # FatFS implementation
    └── diskio_microsd.cpp # MicroSD disk I/O module
```

## Requirements
- PlatformIO
- WeAct STM32H7 board with LCD
- ARM GCC toolchain (included with PlatformIO)
- Board definition file: [`stm32h723weact.json`](stm32h723weact.json) in `/home/$USER/.platformio/platforms/ststm32/`

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

## Code Examples
The project includes a comprehensive [`EXAMPLES.md`](EXAMPLES.md) file that demonstrates practical usage of the RTOS features:

- Task Management
  - Basic task creation and scheduling
  - Dynamic task creation
  - Task synchronization and delays

- Memory Management
  - Memory statistics and monitoring
  - Allocation tracking
  - Memory protection configuration

- Peripheral Usage
  - UART communication
  - ADC temperature monitoring
  - LCD display operations
  - microSD card operations

- Error Handling
  - Basic error handling
  - Custom error recovery
  - Error reporting

- System Configuration
  - MPU configuration
  - Complete system setup

Each example includes working code snippets that can be directly used or adapted for your application. The examples are based on the actual implementation in the codebase and follow the same coding style and conventions.

## License
MIT License - See [LICENSE.md](LICENSE.md) for details. 