#include "syscall.hpp"

#include "middleware/FatFs/ff.h"
#include "peripherals/uart.hpp"
#include "system/scheduler.hpp"

#include <cstdio>
#include <string>

extern "C" {
void SVC_Handler(void) {
  asm volatile("TST LR, #4\n"    // test bit 2 of EXC_RETURN to see if it's a thread mode return
               "ITE EQ\n"        // if it's a thread mode return, skip the next instruction
               "MRSEQ R0, MSP\n" // if it's a main mode return, load MSP into R0
               "MRSNE R0, PSP\n" // if it's a thread mode return, load PSP into R0
               "b svc_handler\n" // Branch to C handler with SP in R0
  );
}

void svc_handler(uint32_t *sp) {
  void *arg0 = (void *)sp[0];
  void *arg1 = (void *)sp[1];
  void *arg2 = (void *)sp[2];
  void *arg3 = (void *)sp[3];
  uint32_t svc_number = sp[4];

  switch (svc_number) {
  case SYS_OPEN:
    *(int *)arg3 = f_open((FIL *)arg0, (char *)arg1, *(uint8_t *)arg2);
    break;
  case SYS_CLOSE:
    *(int *)arg1 = f_close((FIL *)arg0);
    break;
  case SYS_WRITE:
    if (*(int *)arg0 == FILE_STDOUT || *(int *)arg0 == FILE_STDERR) {
      UART::write((char *)arg1, *(int *)arg2);
    } else {
      *(int *)arg3 = f_write((FIL *)arg0, (char *)arg1, *(int *)arg2, nullptr);
    }
    break;
  case SYS_READ:
    if (*(int *)arg0 == FILE_STDIN) {
      // UART::read(arg1, (char *)arg2, arg3);
    } else {
      *(int *)arg3 = f_read((FIL *)arg0, (char *)arg1, *(int *)arg2, nullptr);
    }
    break;
  case SYS_WAITFOR:
    Scheduler::yieldDelay(*(uint32_t *)arg0);
    break;
  default:
    printf("Unknown syscall: %lu\n", svc_number);
    break;
  }
}
};

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