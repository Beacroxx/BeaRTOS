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

#if ENABLE_ALLOCATION_TRACKER
  // Allocations
  struct Allocation {
    void* ptr;
    size_t size;
    const char* file;
    uint32_t line;
  };
#endif

  // Initialize memory tracking
  static void init();

  // Get memory usage statistics
  static void getStats(MemoryRegion& flash, MemoryRegion& ram, MemoryRegion& heap);

  // Override malloc/free to track dynamic allocations
  static void* malloc(size_t size, const char* file = nullptr, uint32_t line = 0);
  static void free(void* ptr, const char* file = nullptr, uint32_t line = 0);
  static void* realloc(void* ptr, size_t size, const char* file = nullptr, uint32_t line = 0);

#if ENABLE_ALLOCATION_TRACKER
  // Print active allocations
  static void printAllocations();
#endif

private:
  // Heap tracking
  static MemoryRegion heap;

#if ENABLE_ALLOCATION_TRACKER
  // Allocation tracking
  static constexpr size_t MAX_ALLOCATIONS = 128;
  static Allocation allocations[MAX_ALLOCATIONS];
  static size_t allocationCount;
#endif
}; 