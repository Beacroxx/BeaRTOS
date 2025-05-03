#include "scheduler.hpp"

#include "memory.hpp"
#include "stm32h7xx_hal.h"
#include <cstdio>
#include <cstring>

// initialize variables
uint32_t Scheduler::taskCount = 0;
TCB *Scheduler::tasks = nullptr;
TCB *Scheduler::currentTask = nullptr;
TCB *Scheduler::nextTask = nullptr;
uint32_t Scheduler::taskIndex = 0;
bool Scheduler::active = false;

void Scheduler::start() {
  if (tasks == nullptr) {
    return;
  }

  active = true;
  currentTask = &tasks[0];
  currentTask->state = TaskState::RUNNING;
  taskIndex = 0;

  asm volatile("LDR r0, =_ZN9Scheduler11currentTaskE\n" // r0 = &Scheduler::currentTask
               "LDR r1, [r0]\n"                         // r1 = currentTask
               "LDR r0, [r1]\n"                         // r0 = currentTask->stackPointer
               "MSR PSP, r0\n"                          // Set PSP to stackPointer
               "MOV r0, #2\n"                           // r0 = 2 (use PSP in thread mode, privileged)
               "MSR CONTROL, r0\n"                      // Set CONTROL register
               "ISB\n"                                  // Instruction Synchronization Barrier
               "POP {r4-r11}\n"                         // Pop callee-saved registers
               "POP {r0-r3}\n"                          // Pop initial registers
               "POP {r12}\n"                            // Pop r12
               "POP {lr}\n"                             // Pop LR
               "POP {pc}\n"                             // Jump to task
  );
}

void Scheduler::initTaskStack(void (*task)(void), uint32_t stackSize, const char *name) {
  __disable_irq();
  // Allocate one task if none exist
  if (tasks == nullptr) {
    tasks = (TCB *)Memory::malloc(sizeof(TCB));
    memset(tasks, 0, sizeof(TCB));
  } else {
    // Reallocate tasks array
    tasks = (TCB *)Memory::realloc(tasks, (taskCount + 1) * sizeof(TCB));
    // only memset the new task
    memset(&tasks[taskCount], 0, sizeof(TCB));
  }

  taskCount++;

  // Allocate stack for new task
  TCB *newTask = &tasks[taskCount - 1];
  newTask->stackBase = (uint32_t *)Memory::malloc(stackSize * sizeof(uint32_t));
  memset(newTask->stackBase, 0, stackSize * sizeof(uint32_t));
  newTask->stackPointer = newTask->stackBase + stackSize;

  // push task context
  *(--newTask->stackPointer) = 0x01000000;         // xPSR
  *(--newTask->stackPointer) = (uint32_t)task;     // PC
  *(--newTask->stackPointer) = (uint32_t)taskExit; // LR
  *(--newTask->stackPointer) = 0x00000000;         // R12
  *(--newTask->stackPointer) = 0x00000000;         // R3
  *(--newTask->stackPointer) = 0x00000000;         // R2
  *(--newTask->stackPointer) = 0x00000000;         // R1
  *(--newTask->stackPointer) = 0x00000000;         // R0

  // Push space for callee-saved registers (R4–R11)
  for (int i = 0; i < 8; ++i)
    *(--newTask->stackPointer) = 0;

  // set task name
  if (name != nullptr) {
    strncpy(newTask->name, name, sizeof(newTask->name));
  }

  // set task ready
  newTask->state = TaskState::READY;

  __enable_irq();
}

void Scheduler::taskExit() {
  __disable_irq();
  // Set the task to TERMINATED
  currentTask->state = TaskState::TERMINATED;

  // Move all tasks after current task one position back
  for (int i = taskIndex; i < taskCount; i++) {
    memcpy(&tasks[i - 1], &tasks[i], sizeof(TCB));
  }

  // Free currentTask's stack
  Memory::free(currentTask->stackBase);

  // reallocate tasks array to new size
  tasks = (TCB *)Memory::realloc(tasks, (taskCount - 1) * sizeof(TCB));

  // Decrement taskCount
  taskCount--;

  // decrement taskIndex if not 0
  if (taskIndex != 0) {
    taskIndex--;
  }

  // update currentTask pointer to nullptr
  currentTask = nullptr;

  // manually select next task
  updateNextTask();

  __enable_irq();

  // yield to next task
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

void Scheduler::yield() {
  if (!active) return; // If scheduler is not active, do nothing
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

void Scheduler::yieldDelay(uint32_t ms) {
  if (!active || tasks == nullptr) {
    // If scheduler is not active or no tasks exist, just use HAL_Delay
    HAL_Delay(ms);
    return;
  }

  // call yield as often as possible during the delay
  uint32_t startTime = HAL_GetTick();
  while (HAL_GetTick() - startTime < ms) {
    yield();
  }
}

void Scheduler::updateNextTask() {
  uint32_t startIndex = taskIndex;
  // find next ready task or continue with current task
  do {
    nextTask = tasks + taskIndex;
    taskIndex = (taskIndex + 1) % taskCount;
    if (taskIndex == startIndex) break;
  } while (nextTask->state != TaskState::READY && nextTask->state != TaskState::RUNNING);
}

__attribute__((naked)) void Scheduler::switchTasks() {
  __asm__ __volatile__(
      "CPSID I\n" // Disable interrupts

      // check if currentTask is nullptr
      "LDR r1, =_ZN9Scheduler11currentTaskE\n" // r1 = &Scheduler::currentTask
      "LDR r2, [r1]\n"                         // r2 = currentTask
      "CMP r2, #0\n"                           // compare currentTask->stackPointer to nullptr
      "BEQ skip_context_save\n"                // if currentTask->stackPointer == nullptr, jump to skip_context_save

      // save r4-r11 on current task's stack
      "MRS r0, psp\n"                          // r0 = current PSP
      "STMDB r0!, {r4-r11}\n"                  // Push r4-r11, decrement sp
      "STR r0, [r2]\n"                         // currentTask->stack_pointer = r0

      // Set currentTask->state to READY
      "MOV r4, #1\n"                           // r4 = 1 (READY state)
      "STR r4, [r2, #8]\n"                     // currentTask->state = READY

      "skip_context_save:\n"

      // Set currentTask to nextTask
      "LDR r3, =_ZN9Scheduler8nextTaskE\n"     // r3 = &Scheduler::nextTask
      "LDR r4, [r3]\n"                         // r4 = nextTask
      "STR r4, [r1]\n"                         // currentTask = nextTask

      // Set nextTask->state to RUNNING
      "MOV r5, #2\n"                           // r5 = 2 (RUNNING state)
      "STR r5, [r4, #8]\n"                     // nextTask->state = RUNNING

      // load r4-r11 from next task's stack
      "LDR r0, [r4]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP

      "ISB\n" // Instruction Synchronization Barrier

      "CPSIE I\n" // Enable interrupts
      
      // return from interrupt
      "BX LR\n"
      :
      :
      : "r0", "r1", "r2", "r3", "r4", "memory");
}