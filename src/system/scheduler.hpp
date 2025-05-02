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
  static constexpr uint32_t IDLE_WINDOW_MS = 4000; // 4 second window
  static uint32_t tasksInYieldDelay; // Number of tasks currently in yieldDelay
  static uint32_t lastIdleCheckTime; // Last time we checked for idle state
  static uint32_t windowStartTime;   // Start of the measurement window
  static uint32_t windowIdleTime;    // Idle time within current window
  static uint32_t windowTotalTime;   // Total time within current window

  static void init();
  static void start();
  static void yield();
  static void initTaskStack(void (*task)(void), uint32_t stackSize, const char *name = nullptr);
  static void taskExit();
  static void updateNextTask();
  static void switchTasks();
  static void switchTasksNoSave();
  static void yieldDelay(uint32_t ms);
  static uint16_t getIdlePercentage();
private:
  static uint32_t taskIndex;
}; 