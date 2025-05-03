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

void Scheduler::initTaskStack(void (*task)(void), uint32_t stackSize, const char *name) {
  __disable_irq();
  // Allocate one task if none exist
  if (tasks == nullptr) {
    tasks = (TCB *)Memory::malloc(sizeof(TCB), __FILE__, __LINE__);
    memset(tasks, 0, sizeof(TCB));
  } else {
    // Reallocate tasks array
    tasks = (TCB *)Memory::realloc(tasks, (taskCount + 1) * sizeof(TCB), __FILE__, __LINE__);
    // only memset the new task
    memset(&tasks[taskCount], 0, sizeof(TCB));
  }

  taskCount++;

  // Allocate stack for new task
  TCB *newTask = &tasks[taskCount - 1];
  newTask->stackBase = (uint32_t *)Memory::malloc(stackSize * sizeof(uint32_t), __FILE__, __LINE__);
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
  Memory::free(currentTask->stackBase, __FILE__, __LINE__);

  // reallocate tasks array to new size
  tasks = (TCB *)Memory::realloc(tasks, (taskCount - 1) * sizeof(TCB), __FILE__, __LINE__);

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