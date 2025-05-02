#include "system/scheduler.hpp"
#include "error/handler.hpp"
#include "system/memory.hpp"
#include <cstdio>
#include <cstring>

// Random character array
char arng[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

// Scheduler variables
TCB *Scheduler::tasks = nullptr;
TCB *Scheduler::currentTask = nullptr;
TCB *Scheduler::nextTask = nullptr;
uint32_t Scheduler::taskIndex = 0;
uint32_t Scheduler::taskCount = 0;
uint32_t Scheduler::tasksInYieldDelay = 0;
uint32_t Scheduler::lastIdleCheckTime = 0;
uint32_t Scheduler::windowStartTime = 0;
uint32_t Scheduler::windowIdleTime = 0;
uint32_t Scheduler::windowTotalTime = 0;

// Initialize the scheduler
void Scheduler::init() {
  tasks = nullptr;
  currentTask = nullptr;
  nextTask = nullptr;
  taskIndex = 0;
  taskCount = 0;
  tasksInYieldDelay = 0;
  lastIdleCheckTime = 0;
  windowStartTime = HAL_GetTick();
  windowIdleTime = 0;
  windowTotalTime = 0;
}

// Start the scheduler
void Scheduler::start() {
  if (taskCount == 0) {
    return; // If no tasks, stay in handler mode
  }

  // Set currentTask to first task
  currentTask = &tasks[0];
  nextTask = currentTask;

  // Set initial PSP to the first task's stack and switch to it
  __asm__ __volatile__(
      "LDR r0, =_ZN9Scheduler11currentTaskE\n" // r0 = &Scheduler::currentTask
      "LDR r0, [r0]\n"                         // r0 = currentTask
      "LDR r0, [r0]\n"    // r0 = currentTask->stack_pointer
      "MSR psp, r0\n"     // Set PSP to task's stack
      "MOV r0, #2\n"      // r0 = 2 (privileged thread mode using PSP)
      "MSR control, r0\n" // Set CONTROL register
      "ISB\n"             // Instruction Synchronization Barrier
      "POP {r4-r11}\n"    // Pop initial context
      "POP {r0-r3}\n"     // Pop initial registers
      "POP {r12}\n"       // Pop r12
      "POP {lr}\n"        // Pop LR
      "POP {pc}\n"        // Jump to task
      :
      :
      : "r0", "memory");
}

// Yield to next task
void Scheduler::yield() {
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

// Initialize task stack
void Scheduler::initTaskStack(void (*task)(void), uint32_t stackSize, const char *name) {
  TCB *tcb = nullptr;

  // Reallocate tasks array
  if (tasks == nullptr) {
    tasks = static_cast<TCB *>(Memory::malloc(sizeof(TCB)));
  } else {
    tasks = static_cast<TCB *>(Memory::realloc(tasks, sizeof(TCB) * (taskCount + 1)));
  }
  tcb = &tasks[taskCount];
  taskCount++;

  // Allocate stack
  tcb->stack = static_cast<uint32_t *>(Memory::malloc(stackSize * sizeof(uint32_t)));
  if (tcb->stack == nullptr) {
    ErrorHandler::hardFault(); // If this task is called, there is a problem
  }

  uint32_t *sp = tcb->stack + stackSize;

  // Push the hardware-saved context (exception frame), top to bottom:
  *(--sp) = 0x01000000;         // xPSR (Thumb state)
  *(--sp) = (uint32_t)task;     // PC (entry point)
  *(--sp) = (uint32_t)taskExit; // LR (what happens if task returns)
  *(--sp) = 0;                  // R12
  *(--sp) = 0;                  // R3
  *(--sp) = 0;                  // R2
  *(--sp) = 0;                  // R1
  *(--sp) = 0;                  // R0 

  // Push space for callee-saved registers (R4–R11)
  for (int i = 0; i < 8; ++i)
    *(--sp) = 0;

  tcb->stack_pointer = sp; // Set initial stack pointer
  tcb->state = TaskState::READY;

  // Generate random name if name is nullptr
  if (name == nullptr) {
    for (int i = 0; i < 16; i++) 
      tcb->name[i] = arng[rand() % (sizeof(arng) - 1)];
  } else
    strcpy(tcb->name, name);
}

// Task exit handler
void Scheduler::taskExit() {
  __disable_irq();
  
  // Mark current task as terminated
  currentTask->state = TaskState::TERMINATED;

  taskCount--;

  // create temporary TCB array
  TCB *tempTasks = static_cast<TCB *>(Memory::malloc(sizeof(TCB) * taskCount));

  // copy tasks excluding terminated tasks
  int j = 0;
  for (int i = 0; i < taskCount + 1; i++) {
    if (tasks[i].state != TaskState::TERMINATED) {
      memcpy(&tempTasks[j], &tasks[i], sizeof(TCB));
      j++;
      // Magic loop that fixes a bug for some reason (I don't know why)
      for (int i = 0; i < 100000; i++) {
        asm volatile("NOP");
      }
    }
  }

  // free current task stack
  Memory::free(currentTask->stack);

  // free old tasks array
  Memory::free(tasks);

  // update tasks pointer
  tasks = tempTasks;
  
  nextTask = &tasks[0];
  taskIndex = 0;

  __enable_irq();
  switchTasksNoSave();
}

// Update next task
void Scheduler::updateNextTask() {
  __disable_irq(); 
  uint32_t startIndex = taskIndex;
  
  // Normal round-robin scheduling
  do {
    taskIndex = (taskIndex + 1) % taskCount;
    nextTask = &tasks[taskIndex];
    
    // If we've checked all tasks and found none ready, stay on current task
    if (taskIndex == startIndex) {
      nextTask = currentTask;
      break;
    }
  } while (nextTask->state != TaskState::READY);
  
  __enable_irq();
}

__attribute__((naked)) void Scheduler::switchTasks() {
  __asm__ __volatile__(
      // save r4-r11 on current task's stack
      "MRS r0, psp\n"                          // r0 = current PSP
      "STMDB r0!, {r4-r11}\n"                  // Push r4-r11, decrement sp
      "LDR r1, =_ZN9Scheduler11currentTaskE\n" // r1 = &Scheduler::currentTask
      "LDR r2, [r1]\n"                         // r2 = currentTask
      "STR r0, [r2]\n"                         // currentTask->stack_pointer = r0

      // Set currentTask to nextTask
      "LDR r3, =_ZN9Scheduler8nextTaskE\n"     // r3 = &Scheduler::nextTask
      "LDR r4, [r3]\n"                         // r4 = nextTask
      "STR r4, [r1]\n"                         // currentTask = nextTask

      // load r4-r11 from next task's stack
      "LDR r0, [r4]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP
      
      // return from interrupt
      "BX LR\n"
      :
      :
      : "r0", "r1", "r2", "r3", "r4", "memory");
}

__attribute__((naked)) void Scheduler::switchTasksNoSave() {
   __asm__ __volatile__(
      // load nextTask
      "LDR r4, =_ZN9Scheduler8nextTaskE\n"     // r4 = &Scheduler::nextTask
      "LDR r4, [r4]\n"                         // r4 = nextTask

      // load r4-r11 from next task's stack
      "LDR r0, [r4]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP
      
      // return from interrupt
      "BX LR\n"
      :
      :
      : "r0", "r1", "r2", "r3", "r4", "memory");
}

void Scheduler::yieldDelay(uint32_t ms) {
  uint32_t startTime = HAL_GetTick();
  uint32_t targetTick = startTime + ms;
  constexpr uint32_t IDLE_CHECK_INTERVAL = 10; // Check every 10ms
  
  // Increment counter for this task entering yieldDelay
  tasksInYieldDelay++;
  lastIdleCheckTime = startTime;
  uint32_t nextIdleCheck = startTime + IDLE_CHECK_INTERVAL;
  
  while (HAL_GetTick() < targetTick) {
    if (nextTask != nullptr && taskCount > 1) {
      uint32_t currentTime = HAL_GetTick();
      
      // Only check idle time periodically
      if (currentTime >= nextIdleCheck) {
        uint32_t activeTasks = taskCount;
        uint32_t deltaTime = currentTime - lastIdleCheckTime;
        
        // Check if we need to reset the window
        if (currentTime - windowStartTime >= IDLE_WINDOW_MS) {
          windowStartTime = currentTime;
          windowIdleTime = 0;
          windowTotalTime = 0;
        }
        
        // Accumulate time in current window
        windowTotalTime += deltaTime;
        
        if (tasksInYieldDelay == activeTasks) {
          windowIdleTime += deltaTime;
        }
        lastIdleCheckTime = currentTime;
        nextIdleCheck = currentTime + IDLE_CHECK_INTERVAL;
      }
      
      yield();
    }
  }
  
  // Decrement counter when task exits yieldDelay
  tasksInYieldDelay--;
}

uint16_t Scheduler::getIdlePercentage() {
  if (windowTotalTime == 0) {
    return 0;
  }
  // Calculate percentage and scale to 16-bit range (0-65535)
  uint32_t percentage = (uint32_t)(((uint64_t)windowIdleTime * 65535) / windowTotalTime);
  return (uint16_t)percentage;
}