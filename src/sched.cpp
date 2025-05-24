#include "main.h"

namespace SCHED {
uint32_t taskCount = 0;
const uint32_t TCB_SIZE = sizeof(TCB);
TCB *tasks = nullptr;
TCB *currentTask = nullptr;
TCB *nextTask = nullptr;
bool active = false;
uint32_t taskIndex = 0;

void updateNextTask() {
  uint32_t startIndex = taskIndex;
  do {
    nextTask = tasks + taskIndex;
    taskIndex = (taskIndex + 1) % taskCount;
    if (taskIndex == startIndex) break;
  } while (nextTask->state != TaskState::READY && nextTask->state != TaskState::RUNNING);
}

void taskExit() {
  cm_disable_interrupts();

  currentTask->state = TaskState::TERMINATED;

  for (uint i = taskIndex; i < taskCount; i++) {
    memcpy(&tasks[i - 1], &tasks[i], sizeof(TCB));
  }

  free(currentTask->stackBase);
  tasks = (TCB *)realloc(tasks, (taskCount - 1) * sizeof(TCB));

  taskCount--;
  if (taskIndex != 0) {
    taskIndex--;
  }

  currentTask = nullptr;

  updateNextTask();
  cm_enable_interrupts();
  yield();
}

void initTaskStack(void (*task)(void), uint32_t stackSize, const char *name) {
  cm_disable_interrupts();

  if (tasks == nullptr) {
    tasks = (TCB *)malloc(sizeof(TCB));
    memset(tasks, 0, sizeof(TCB));
  } else {
    tasks = (TCB *)realloc(tasks, (taskCount + 1) * sizeof(TCB));
    memset(&tasks[taskCount], 0, sizeof(TCB));
  }

  taskCount++;

  TCB *newTask = &tasks[taskCount - 1];
  newTask->stackBase = (uint32_t *)malloc(stackSize * sizeof(uint32_t));
  memset(newTask->stackBase, 0, stackSize * sizeof(uint32_t));
  newTask->stackPointer = newTask->stackBase + stackSize;

  *(--newTask->stackPointer) = 0x01000000;         // xPSR
  *(--newTask->stackPointer) = (uint32_t)task;     // PC
  *(--newTask->stackPointer) = (uint32_t)taskExit; // LR
  *(--newTask->stackPointer) = 0x00000000;         // R12
  *(--newTask->stackPointer) = 0x00000000;         // R3
  *(--newTask->stackPointer) = 0x00000000;         // R2
  *(--newTask->stackPointer) = 0x00000000;         // R1
  *(--newTask->stackPointer) = 0x00000000;         // R0

  for (int i = 0; i < 8; ++i)
    *(--newTask->stackPointer) = 0;

  if (name != nullptr) {
    strncpy(newTask->name, name, sizeof(newTask->name));
  }

  newTask->state = TaskState::READY;
  cm_enable_interrupts();
}
} // namespace SCHED 

extern "C" {
// PendSV ISR
void pend_sv_handler(void) {
  if (!SCHED::active) return;
  SCHED::updateNextTask();
  SCHED::switchTasks();
}

// Error Handler
void cm3_assert_failed(void) {
  while (1) {
    GPIO::toggleLed();
    // busy wait loop
    volatile uint32_t i = 0;
    while (++i < 1000000)
      ;
  }
}
}