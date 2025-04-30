#include "system/scheduler.hpp"
#include "error/handler.hpp"
#include <cstdio>

TCB *Scheduler::tasks = nullptr;
TCB *Scheduler::currentTask = nullptr;
TCB *Scheduler::nextTask = nullptr;
uint32_t Scheduler::taskIndex = 0;
uint32_t Scheduler::taskCount = 0;

void Scheduler::init() {
  tasks = (TCB *)malloc(sizeof(TCB)); // allocate just one TCB for dummy task
  Scheduler::initTaskStack([]() {
    ErrorHandler::hardFault(); // If this task is called, there is a problem
  }, 32);
  currentTask = nextTask = &tasks[0];
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

void Scheduler::initTaskStack(void (*task)(void), uint32_t stackSize) {

  // look for terminated tasks to reclaim
  int reclaimed = -1;
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].state == TaskState::TERMINATED) {
      free(tasks[i].stack);
      tasks[i].state = TaskState::UNINITIALIZED;
      reclaimed = i;
      break;
    }
  }

  TCB *tcb = nullptr;

  // increase tasks array only if there are no uninitialized tasks to reclaim
  if (reclaimed == -1) {
    tasks = (TCB *)realloc(tasks, sizeof(TCB) * (taskCount + 1));
    tcb = &tasks[taskCount];
    taskCount++;
  } else {
    tcb = &tasks[reclaimed];
  }

  // Allocate stack 
  tcb->stack = (uint32_t *)malloc(stackSize * sizeof(uint32_t));
  if (tcb->stack == nullptr) {
    ErrorHandler::hardFault(); // If this task is called, there is a problem
  }

  uint32_t *sp = tcb->stack + stackSize;

  // Push the hardware-saved context (exception frame), top to bottom:
  *(--sp) = 0x01000000;         // xPSR (Thumb state)
  *(--sp) = (uint32_t)task;     // PC (entry point)
  *(--sp) = (uint32_t)taskExit; // LR (what happens if task returns)
  *(--sp) = 0;                  // R12
  *(--sp) = 0;                  // R3
  *(--sp) = 0;                  // R2
  *(--sp) = 0;                  // R1
  *(--sp) = (uint32_t)tcb;     // R0 (pointer to TCB) for taskExit

  // Push space for callee-saved registers (R4–R11)
  for (int i = 0; i < 8; ++i)
    *(--sp) = 0;

  tcb->stack_pointer = sp; // Set initial stack pointer
  tcb->state = TaskState::READY;
}

void Scheduler::taskExit(uint32_t *tcbptr) {
  TCB *tcb = (TCB *)tcbptr;
  free(tcb->stack);
  tcb->state = TaskState::TERMINATED;
  printf("Task exited\n");
}

void Scheduler::updateNextTask() {
  // find next ready or running task
  do {
    taskIndex = (taskIndex + 1) % (taskCount);
    nextTask = &tasks[taskIndex];
  } while (nextTask->state != TaskState::READY && nextTask->state != TaskState::RUNNING);
}

__attribute__((naked)) void Scheduler::switchTasks() {
  __asm__ __volatile__(
      // save r4-r11 on current task's stack
      "MRS r0, psp\n"                          // r0 = current PSP
      "STMDB r0!, {r4-r11}\n"                  // Push r4-r11, decrement sp
      "LDR r1, =_ZN9Scheduler11currentTaskE\n" // r1 = &Scheduler::currentTask
      "LDR r2, [r1]\n"                         // r2 = currentTask
      "STR r0, [r2]\n"                         // currentTask->stack_pointer = r0

      // Set currentTask to nextTask
      "LDR r3, =_ZN9Scheduler8nextTaskE\n" // r3 = &Scheduler::nextTask
      "LDR r4, [r3]\n"                      // r4 = nextTask
      "STR r4, [r1]\n"                      // currentTask = nextTask

      // load r4-r11 from next task's stack
      "LDR r0, [r4]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP

      // return from interrupt
      "BX LR\n"
      :
      : 
      : "memory"
      );
}