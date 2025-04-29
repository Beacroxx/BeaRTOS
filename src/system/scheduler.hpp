#pragma once

#include <cstdint>

#ifndef TASKS
#define TASKS 0 // 0 for no tasks
#endif

struct TCB {
  uint32_t *stack_pointer;
  uint32_t stack[1024];
};

class Scheduler {
public:
  static constexpr uint32_t TASK_COUNT = TASKS;
  static constexpr uint32_t TCB_SIZE = sizeof(TCB);
  static TCB tasks[TASK_COUNT + 1]; // +1 for the dummy task
  static TCB *currentTask;

  static void init();
  static void start();
  static void yield();
  static void initTaskStack(TCB &tcb, void (*task)(void));
  static void switchTasks();

private:
  static uint32_t taskIndex;
  static void taskExit();
}; 