#pragma once

#include <cstdint>

// Task state enum
enum class TaskState { UNINITIALIZED, READY, RUNNING, SUSPENDED, TERMINATED };

// Task Control Block
struct TCB {
  uint32_t *stackPointer;
  uint32_t *stackBase;
  TaskState state;
  char name[16];
} __attribute__((aligned(32)));

// Scheduler namespace
namespace Scheduler {
// Scheduler variables
extern uint32_t taskCount;
constexpr uint32_t TCB_SIZE = sizeof(TCB);
extern TCB *tasks;
extern TCB *currentTask;
extern TCB *nextTask;
extern bool active;
constexpr uint32_t IDLE_WINDOW_MS = 4000; // 4 second window
extern uint32_t tasksInYieldDelay;        // Number of tasks currently in yieldDelay
extern uint32_t lastIdleCheckTime;        // Last time we checked for idle state
extern uint32_t windowStartTime;          // Start of the measurement window
extern uint32_t windowIdleTime;           // Idle time within current window
extern uint32_t windowTotalTime;          // Total time within current window

void start();
void yield();
void initTaskStack(void (*task)(void), uint32_t stackSize, const char *name = nullptr);
void taskExit();
void updateNextTask();
void switchTasks();
void yieldDelay(uint32_t ms);

extern uint32_t taskIndex;
} // namespace Scheduler