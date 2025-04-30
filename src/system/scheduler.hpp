#pragma once

#include <cstdint>

// Task state enum
enum class TaskState {
  UNINITIALIZED,
  READY,
  SUSPENDED,
  TERMINATED
};

// Task Control Block
struct TCB {
  uint32_t *stack_pointer;
  uint32_t *stack;
  TaskState state;
  char name[16];
};

// Scheduler class
class Scheduler {
public:
  // Scheduler variables
  static uint32_t taskCount;
  static constexpr uint32_t TCB_SIZE = sizeof(TCB);
  static TCB *tasks;
  static TCB *currentTask;
  static TCB *nextTask;

  static void init();
  static void start();
  static void yield();
  static void initTaskStack(void (*task)(void), uint32_t stackSize, const char *name = nullptr);
  static void taskExit();
  static void updateNextTask();
  static void switchTasks();
  static void yieldDelay(uint32_t ms);
private:
  static uint32_t taskIndex;
}; 