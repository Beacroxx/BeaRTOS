#include "memory.hpp"
#include "error/handler.hpp"
#include <cstring>

// Initialize static members
Memory::MemoryRegion Memory::heap = {0, 0, 0};

void Memory::init() {
  // Calculate heap size from linker symbols
  heap.start = reinterpret_cast<uint32_t>(&_ebss);
  heap.size = reinterpret_cast<uint32_t>(&_estack) - heap.start;
  heap.used = 0;
}

void Memory::getStats(MemoryRegion& flash, MemoryRegion& ram, MemoryRegion& heap) {
  // Flash usage (code + data)
  flash.start = 0x08000000;  // Start of flash
  flash.size = 1024 * 1024;  // 1MB flash
  flash.used = reinterpret_cast<uint32_t>(&_sidata) - flash.start;

  // RAM usage (data + bss)
  ram.start = 0x24000000;  // Start of RAM_D1
  ram.size = 320 * 1024;   // 320KB RAM
  ram.used = reinterpret_cast<uint32_t>(&_ebss) - reinterpret_cast<uint32_t>(&_sdata);

  // Heap usage
  heap = Memory::heap;
}

void* Memory::malloc(size_t size) {
  void* ptr = ::malloc(size);
  if (ptr) {
    heap.used += size;
  }
  return ptr;
}

void Memory::free(void* ptr) {
  if (ptr) {
    // Note: This is a simplified implementation. In a real system,
    // you'd need to track the actual size of each allocation.
    // For now, we'll just decrement by a fixed amount to show the concept.
    heap.used -= 4;  // Assuming 4 bytes per allocation
    ::free(ptr);
  }
}

void* Memory::realloc(void* ptr, size_t size) {
  void* new_ptr = ::realloc(ptr, size);
  if (new_ptr) {
    if (ptr) {
      heap.used -= 4;  // Free old allocation
    }
    heap.used += size;  // Add new allocation
  }
  return new_ptr;
} 