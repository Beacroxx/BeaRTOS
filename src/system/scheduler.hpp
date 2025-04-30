#pragma once

#include <cstdint>

enum class TaskState {
  UNINITIALIZED,
  READY,
  RUNNING,
  SUSPENDED,
  TERMINATED
};

struct TCB {
  uint32_t *stack_pointer;
  uint32_t *stack;
  TaskState state;
};

class Scheduler {
public:
  static uint32_t taskCount;
  static constexpr uint32_t TCB_SIZE = sizeof(TCB);
  static TCB *tasks;
  static TCB *currentTask;
  static TCB *nextTask;

  static void init();
  static void start();
  static void yield();
  static void initTaskStack(void (*task)(void), uint32_t stackSize);
  static void switchTasks();
  static void updateNextTask();
private:
  static uint32_t taskIndex;
  static void taskExit(uint32_t *tcbptr);
}; 