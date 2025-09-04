#pragma once

// Standard C++ headers
#include <cstdarg>
#include <cstdint>
#include <cstdio>

enum {
  SYS_WRITE,
  SYS_READ,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_LSEEK,
  SYS_STAT,
  SYS_UNLINK,
  SYS_EXECVE,
  SYS_WAITFOR,
};

#define FILE_STDIN  0
#define FILE_STDOUT 1
#define FILE_STDERR 2

int32_t syscall(uint8_t svc_number, void *arg0, void *arg1, void *arg2, void *arg3) {
  int32_t result;
  asm volatile("MOV r0, %1 \n"
               "MOV r1, %2 \n"
               "MOV r2, %3 \n"
               "MOV r3, %4 \n"
               "MOV r12, %5 \n"
               "SVC 0      \n"
               "MOV %0, r0   \n"
               : "=r"(result)
               : "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(svc_number)
               : "r0", "r1", "r2", "r3", "r12", "memory");
  return result;
}

// C++ support functions
extern "C" {

int _write(int fd, const char *buf, int count) {
  syscall(SYS_WRITE, &fd, (void *)buf, &count, 0);
  return count;
}

int _read(int fd, char *buf, int count) {
  syscall(SYS_READ, &fd, (void *)buf, &count, 0);
  return count;
}

int _close(int fd) {
  int result;
  syscall(SYS_CLOSE, &fd, &result, 0, 0);
  return result;
}

int _lseek(int fd, int offset, int whence) {
  int result;
  syscall(SYS_LSEEK, &fd, &offset, &whence, &result);
  return result;
}

void *_sbrk(int incr) { return (void *)-1; }

int _fstat(int fd, void *st) { return 0; }

int _isatty(int fd) { return (fd == FILE_STDIN || fd == FILE_STDOUT || fd == FILE_STDERR) ? 1 : 0; }

void _exit(int status) {
  while (1) {
    // Infinite loop - embedded systems don't typically exit
  }
}

int _kill(int pid, int sig) {
  return -1; // Not supported in embedded systems
}

int _getpid(void) {
  return 1; // Return a dummy PID
}

void sleep(uint32_t ms) { syscall(SYS_WAITFOR, &ms, 0, 0, 0); }
}

// This is the entry point that will be called when the executable is loaded
extern "C" __attribute__((section(".text.entry"))) int main();