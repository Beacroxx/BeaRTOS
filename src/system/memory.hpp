#pragma once

#include <cstdint>
#include <cstdlib>

// Linker symbols for memory regions
extern "C" {
  extern uint32_t _sidata;  // Start of initialized data in flash
  extern uint32_t _sdata;   // Start of data section in RAM
  extern uint32_t _edata;   // End of data section in RAM
  extern uint32_t _sbss;    // Start of bss section in RAM
  extern uint32_t _ebss;    // End of bss section in RAM
  extern uint32_t _estack;  // End of stack
}

class Memory {
public:
  // Memory regions from linker script
  struct MemoryRegion {
    uint32_t start;
    uint32_t size;
    uint32_t used;
  };

  // Initialize memory tracking
  static void init();

  // Get memory usage statistics
  static void getStats(MemoryRegion& flash, MemoryRegion& ram, uint32_t& heapUsed, uint32_t& heapFree);

  // Override malloc/free to track dynamic allocations
  static void* malloc(size_t size);
  static void free(void* ptr);
  static void* realloc(void* ptr, size_t size);

private:
  // Heap tracking
  static uint32_t heapUsed;
  static uint32_t heapFree;
  static uint32_t heapTotal;
}; 