#pragma once

#include <cstdint>

// Task state enum
enum class TaskState {
  UNINITIALIZED,
  READY,
  RUNNING,
  SUSPENDED,
  TERMINATED
};

// Task Control Block
struct TCB {
  uint32_t *stackPointer;
  uint32_t *stackBase;
  TaskState state;
  char name[16];
} __attribute__((aligned(32)));

// Scheduler class
class Scheduler {
public:
  // Scheduler variables
  static uint32_t taskCount;
  static constexpr uint32_t TCB_SIZE = sizeof(TCB);
  static TCB *tasks;
  static TCB *currentTask;
  static TCB *nextTask;
  static bool active;
  static constexpr uint32_t IDLE_WINDOW_MS = 4000; // 4 second window
  static uint32_t tasksInYieldDelay; // Number of tasks currently in yieldDelay
  static uint32_t lastIdleCheckTime; // Last time we checked for idle state
  static uint32_t windowStartTime;   // Start of the measurement window
  static uint32_t windowIdleTime;    // Idle time within current window
  static uint32_t windowTotalTime;   // Total time within current window

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