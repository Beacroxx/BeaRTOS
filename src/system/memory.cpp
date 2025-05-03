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

// Magic number to identify our allocations
constexpr size_t MEMORY_MAGIC = 0xDEADBEEF;

void* Memory::malloc(size_t size) {
  if (size > SIZE_MAX - sizeof(size_t) - sizeof(size_t)) return nullptr; // overflow check
  void* ptr = ::malloc(size + sizeof(size_t) + sizeof(size_t));
  if (ptr) {
    heap.used += size;
    *reinterpret_cast<size_t*>(ptr) = size | 0x10000000;
    *reinterpret_cast<size_t*>(static_cast<char*>(ptr) + sizeof(size_t)) = MEMORY_MAGIC;
  }
  return ptr ? static_cast<char*>(ptr) + sizeof(size_t) : nullptr;
}

void Memory::free(void* ptr) {
  if (!ptr) return;

  void* meta_ptr = static_cast<char*>(ptr) - sizeof(size_t);
  // Check magic number first (optional, but recommended)
  size_t magic = *reinterpret_cast<size_t*>(ptr);
  if (magic != MEMORY_MAGIC) {
    // Not our allocation, pass to standard free
    ::free(ptr);
    return;
  }
  // Now check marker bit
  size_t stored = *reinterpret_cast<size_t*>(meta_ptr);
  if ((stored & 0x10000000) == 0) {
    // Not our allocation, pass to standard free
    ::free(ptr);
    return;
  }
  // All checks passed
  size_t size = stored & 0x7FFFFFFF;
  heap.used -= size;
  ::free(meta_ptr);
}

void* Memory::realloc(void* ptr, size_t size) {
  if (size > SIZE_MAX - sizeof(size_t) - sizeof(size_t)) return nullptr;
  if (!ptr) return Memory::malloc(size);

  void* meta_ptr = static_cast<char*>(ptr) - sizeof(size_t);
  // Check magic number first
  size_t magic = *reinterpret_cast<size_t*>(ptr);
  if (magic != MEMORY_MAGIC) {
    // Not our allocation, pass to standard realloc
    return ::realloc(ptr, size);
  }
  // Now check marker bit
  size_t stored = *reinterpret_cast<size_t*>(meta_ptr);
  if ((stored & 0x10000000) == 0) {
    // Not our allocation, pass to standard realloc
    return ::realloc(ptr, size);
  }
  // All checks passed
  size_t old_size = stored & 0x7FFFFFFF;
  void* new_meta_ptr = ::realloc(meta_ptr, size + sizeof(size_t) + sizeof(size_t));
  if (new_meta_ptr) {
    heap.used -= old_size;
    heap.used += size;
    *reinterpret_cast<size_t*>(new_meta_ptr) = size | 0x10000000;
    *reinterpret_cast<size_t*>(static_cast<char*>(new_meta_ptr) + sizeof(size_t)) = MEMORY_MAGIC;
    return static_cast<char*>(new_meta_ptr) + sizeof(size_t);
  }
  return nullptr;
}
