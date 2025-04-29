#include "system/scheduler.hpp"
#include "error/handler.hpp"
#include <cstdio>

TCB Scheduler::tasks[Scheduler::TASK_COUNT + 1];
TCB *Scheduler::currentTask = nullptr;
uint32_t Scheduler::taskIndex = 0;

void Scheduler::init() {
  taskIndex = 0;
  Scheduler::initTaskStack(tasks[TASK_COUNT], []() {
    ErrorHandler::hardFault(); // If this task is called, there is a problem
  });
  currentTask = &tasks[TASK_COUNT];
}

void Scheduler::start() {
  // Set initial PSP to the first task's stack
  __asm__ __volatile__("LDR r0, =_ZN9Scheduler11currentTaskE\n" // r0 = &Scheduler::currentTask
                       "LDR r0, [r0]\n"                         // r0 = currentTask
                       "LDR r0, [r0]\n"                         // r0 = currentTask->stack_pointer
                       "MSR psp, r0\n"                          // Set PSP to task's stack
                       "MOV r0, #2\n"                           // r0 = 2 (privileged thread mode using PSP)
                       "MSR control, r0\n"                      // Set CONTROL register
                       "ISB\n"                                  // Instruction Synchronization Barrier
                       "BX lr\n");
}

void Scheduler::yield() { __asm__ __volatile__("SVC #0"); }

void Scheduler::initTaskStack(TCB &tcb, void (*task)(void)) {
  uint32_t *sp = tcb.stack + 1024;

  // Push the hardware-saved context (exception frame), top to bottom:
  *(--sp) = 0x01000000;         // xPSR (Thumb state)
  *(--sp) = (uint32_t)task;     // PC (entry point)
  *(--sp) = (uint32_t)taskExit; // LR (what happens if task returns)
  *(--sp) = 0;                  // R12
  *(--sp) = 0;                  // R3
  *(--sp) = 0;                  // R2
  *(--sp) = 0;                  // R1
  *(--sp) = 0;                  // R0

  // Push space for callee-saved registers (R4–R11)
  for (int i = 0; i < 8; ++i)
    *(--sp) = 0;

  tcb.stack_pointer = sp; // Set initial stack pointer
}

void Scheduler::taskExit() { printf("Task exited\n"); }

__attribute__((naked)) void Scheduler::switchTasks() {
  __asm__ __volatile__(
      // save r4-r11 on current task's stack
      "MRS r0, psp\n"                          // r0 = current PSP
      "STMDB r0!, {r4-r11}\n"                  // Push r4-r11, decrement sp
      "LDR r1, =_ZN9Scheduler11currentTaskE\n" // r1 = &Scheduler::currentTask
      "LDR r2, [r1]\n"                         // r2 = currentTask
      "STR r0, [r2]\n"                         // currentTask->stack_pointer = r0

      // increment taskIndex and wrap
      "LDR r3, =_ZN9Scheduler9taskIndexE\n" // r3 = &Scheduler::taskIndex
      "LDR r4, [r3]\n"                      // r4 = taskIndex
      "ADD r4, r4, #1\n"                    // r4 = taskIndex + 1
      "MOV r5, %[TASKCOUNT]\n"              // r5 = TASK_COUNT
      "CMP r4, r5\n"                        // compare taskIndex and TASK_COUNT
      "BLT 1f\n"                            // if taskIndex < TASK_COUNT, skip next two instructions
      "MOV r4, #0\n"                        // r4 = 0
      "1:\n"                                // label
      "STR r4, [r3]\n"                      // taskIndex = r4

      // set currentTask = &tasks[taskIndex]
      "LDR r6, =_ZN9Scheduler5tasksE\n" // r6 = &Scheduler::tasks
      "MOV r7, %[TCBSIZE]\n"            // r7 = sizeof(TCB) = 4100 bytes (1024 words + 1 pointer)
      "MUL r7, r4, r7\n"                // r7 = taskIndex * 4100 bytes
      "ADD r6, r6, r7\n"                // r6 = &tasks[taskIndex]
      "STR r6, [r1]\n"                  // currentTask = &tasks[taskIndex]

      // load r4-r11 from next task's stack
      "LDR r0, [r6]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP

      // return from interrupt
      "BX LR\n"
      :
      : [TASKCOUNT] "i"(TASK_COUNT), [TCBSIZE] "i"(TCB_SIZE));
}