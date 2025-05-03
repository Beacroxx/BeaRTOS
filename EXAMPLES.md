# STM32H7 RTOS Code Examples

This document provides practical code examples for using the various features of the STM32H7 RTOS.

## Task Management

### Creating a Basic Task
```cpp
void myTask(void) {
  while (1) {
    printf("Hello from myTask!\n");
    Scheduler::yieldDelay(1000); // Delay for 1 second
  }
}

// Initialize the task with a 256-word stack
Scheduler::initTaskStack(myTask, 256, "myTask");
```

### Task with Dynamic Creation
```cpp
void parentTask(void) {
  printf("Parent task starting\n");
  Scheduler::yieldDelay(500);
  
  // Create a child task
  Scheduler::initTaskStack(childTask, 256, "childTask");
  
  while (1) {
    printf("Parent task running\n");
    Scheduler::yieldDelay(1000);
  }
}

void childTask(void) {
  while (1) {
    printf("Child task running\n");
    Scheduler::yieldDelay(500);
  }
}
```

## Memory Management

### Memory Statistics
```cpp
void memoryStatsTask(void) {
  while (1) {
    Memory::MemoryRegion flash, ram, heap;
    Memory::getStats(flash, ram, heap);
    
    printf("Flash: %lu/%lu bytes used\n", flash.used, flash.size);
    printf("RAM: %lu/%lu bytes used\n", ram.used, ram.size);
    printf("Heap: %lu/%lu bytes used\n", heap.used, heap.size);
    
    Scheduler::yieldDelay(1000);
  }
}
```

### Memory Allocation with Tracking
```cpp
void allocationDemo(void) {
  while (1) {
    // Allocate memory with tracking
    void* ptr = Memory::malloc(1024, __FILE__, __LINE__);
    
    if (ptr) {
      printf("Allocated 1KB at %p\n", ptr);
      
      #if ENABLE_ALLOCATION_TRACKER
      Memory::printAllocations();
      #endif
      
      Memory::free(ptr, __FILE__, __LINE__);
    }
    
    Scheduler::yieldDelay(2000);
  }
}
```

## Peripheral Usage

### UART Communication
```cpp
void uartTask(void) {
  while (1) {
    printf("Sending data over UART\n");
    Scheduler::yieldDelay(1000);
  }
}
```

### ADC Temperature Monitoring
```cpp
void temperatureTask(void) {
  while (1) {
    float temp = ADC::getTemperature();
    printf("Temperature: %.2f°C\n", temp);
    Scheduler::yieldDelay(1000);
  }
}
```

### LCD Display
```cpp
#if ENABLE_LCD
void lcdTask(void) {
  while (1) {
    // Clear screen
    LCD::fillRect(0, 0, LCD::WIDTH, LCD::HEIGHT, BLACK);
    
    // Draw text
    LCD::drawString(0, 0, 12, "System Info");
    LCD::drawString(0, 12, 12, "CPU Temp: XX°C");
    
    // Update display
    LCD::update();
    
    Scheduler::yieldDelay(50);
  }
}
#endif
```

### microSD Card Operations
```cpp
#if ENABLE_MICROSD 
void sdCardTask(void) {
  if (MicroSD::available()) {
    uint64_t cardInfo = MicroSD::getCardInfo();
    uint32_t cardSize = cardInfo >> 32;
    uint32_t cardBlockSize = cardInfo & 0xFFFFFFFF;
    
    printf("Card size: %lu blocks, %lu bytes per block\n", 
           cardSize, cardBlockSize);
  }
  
  while (1) {
    // Read/write operations
    uint8_t buffer[512];
    MicroSD::readBlocks(buffer, 0, 1, 1000); // Read first block
    MicroSD::writeBlocks(buffer, 1, 1, 1000); // Write to second block
    
    Scheduler::yieldDelay(5000);
  }
}
#endif
```

## Error Handling

### Basic Error Handling
```cpp
void errorDemoTask(void) {
  // Example of error handling
  if (someCondition) {
    ErrorHandler::handle(ErrorCode::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
  }
  
  while (1) {
    Scheduler::yieldDelay(1000);
  }
}
```

### Custom Error Recovery
```cpp
void recoverableTask(void) {
  while (1) {
		// Attempt some operation
		if (operationFails) {
			ErrorHandler::handle(ErrorCode::SD_CARD_WRITE_FAILED, __FILE__, __LINE__);
			// Code here will execute if error is not critical
			printf("Recovered from MicroSD write failure\n");
		}
    
    Scheduler::yieldDelay(1000);
  }
}
```

## System Configuration

### MPU Configuration
```cpp
void Init_MPU() {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  HAL_MPU_Disable();

  // Configure AXI SRAM region
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
```

## Complete System Setup

```cpp
int main(void) {
  // Initialize HAL
  if (HAL_Init() != HAL_OK) {
    ErrorHandler::handle(ErrorCode::HAL_INIT_FAILED, __FILE__, __LINE__);
  }
  
  // Configure MPU
  Init_MPU();
  
  // Enable caches
  SCB_EnableICache();
  SCB_EnableDCache();
  
  // Initialize system components
  SystemClock::init();
  SystemTick::init();
  GPIO::init();
  UART::init();
  Memory::init();
  SPI::init();
  Timer::init();
  
  // Initialize optional peripherals
  #if ENABLE_MICROSD
  MicroSD::init();
  #endif
  
  #if ENABLE_LCD
  LCD::init();
  #endif
  
  // Initialize ADC
  ADC::init();
  ADC::calibrate();
  
  // Create tasks
  Scheduler::initTaskStack(memoryStatsTask, 256, "memStats");
  Scheduler::initTaskStack(temperatureTask, 256, "temp");
  #if ENABLE_LCD
  Scheduler::initTaskStack(lcdTask, 256, "lcd");
  #endif
  
  // Start scheduler
  Scheduler::start();
  
  // Should never reach here
  ErrorHandler::handle(ErrorCode::UNKNOWN, __FILE__, __LINE__);
}
``` 