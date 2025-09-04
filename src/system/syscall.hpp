#pragma once

#include <cstdint>

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

int32_t syscall(uint8_t svc_number, void *arg0, void *arg1, void *arg2, void *arg3);