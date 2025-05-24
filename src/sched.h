#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace SCHED {
  enum class TaskState { UNINITIALIZED, READY, RUNNING, SUSPENDED, TERMINATED };

  struct TCB {
    uint32_t *stackPointer;
    uint32_t *stackBase;
    TaskState state;
    char name[16];
  } __attribute__((aligned(32)));

  extern uint32_t taskCount;
  extern const uint32_t TCB_SIZE;
  extern TCB *tasks;
  extern TCB *currentTask;
  extern TCB *nextTask;
  extern bool active;
  extern uint32_t taskIndex;

  void start();
  void switchTasks();
  void updateNextTask();
  void taskExit();
  void initTaskStack(void (*task)(void), uint32_t stackSize, const char *name);
} 