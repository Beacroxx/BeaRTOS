#include "memory.hpp"
#include "error/handler.hpp"
#include <cstring>

// Initialize static members
uint32_t Memory::heapUsed = 0;
uint32_t Memory::heapFree = 0;
uint32_t Memory::heapTotal = 0;

void Memory::init() {
  // Calculate heap size from linker symbols
  heapTotal = reinterpret_cast<uint32_t>(&_estack) - reinterpret_cast<uint32_t>(&_ebss);
  heapFree = heapTotal;
}

void Memory::getStats(MemoryRegion& flash, MemoryRegion& ram, uint32_t& heapUsed, uint32_t& heapFree) {
  // Flash usage (code + data)
  flash.start = 0x08000000;  // Start of flash
  flash.size = 1024 * 1024;  // 1MB flash
  flash.used = reinterpret_cast<uint32_t>(&_sidata) - flash.start;

  // RAM usage (data + bss)
  ram.start = 0x24000000;  // Start of RAM_D1
  ram.size = 320 * 1024;   // 320KB RAM
  ram.used = reinterpret_cast<uint32_t>(&_ebss) - reinterpret_cast<uint32_t>(&_sdata);

  // Heap usage
  heapUsed = Memory::heapUsed;
  heapFree = Memory::heapFree;
}

void* Memory::malloc(size_t size) {
  void* ptr = ::malloc(size);
  if (ptr) {
    heapUsed += size;
    heapFree -= size;
  }
  return ptr;
}

void Memory::free(void* ptr) {
  if (ptr) {
    // Note: This is a simplified implementation. In a real system,
    // you'd need to track the actual size of each allocation.
    // For now, we'll just decrement by a fixed amount to show the concept.
    heapUsed -= 4;  // Assuming 4 bytes per allocation
    heapFree += 4;
    ::free(ptr);
  }
}

void* Memory::realloc(void* ptr, size_t size) {
  void* new_ptr = ::realloc(ptr, size);
  if (new_ptr) {
    if (ptr) {
      heapUsed -= 4;  // Free old allocation
      heapFree += 4;
    }
    heapUsed += size;  // Add new allocation
    heapFree -= size;
  }
  return new_ptr;
} 