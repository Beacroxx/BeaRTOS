#include "system/scheduler.hpp"
#include "error/handler.hpp"
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

// Initialize the scheduler
void Scheduler::init() {
  // allocate a dummy task that will get overwritten by the first task
  tasks = static_cast<TCB *>(malloc(sizeof(TCB))); // allocate just one TCB for dummy task
  currentTask = nextTask = &tasks[0]; // Set current and next task to dummy task
}

// Start the scheduler
void Scheduler::start() {
  if (currentTask == nullptr) {
    return; // If no tasks, stay in handler mode
  }

  // Set initial PSP to the first task's stack
  __asm__ __volatile__(
      "LDR r0, =_ZN9Scheduler11currentTaskE\n" // r0 = &Scheduler::currentTask
      "LDR r0, [r0]\n"                         // r0 = currentTask
      "LDR r0, [r0]\n"    // r0 = currentTask->stack_pointer
      "MSR psp, r0\n"     // Set PSP to task's stack
      "MOV r0, #2\n"      // r0 = 2 (privileged thread mode using PSP)
      "MSR control, r0\n" // Set CONTROL register
      "ISB\n"             // Instruction Synchronization Barrier
      "BX lr\n");
}

// Yield to next task
void Scheduler::yield() { __asm__ __volatile__("SVC #0"); }

// Initialize task stack
void Scheduler::initTaskStack(void (*task)(void), uint32_t stackSize,
                              const char *name) {
  // look for terminated tasks to reclaim
  int reclaimed = -1;
  for (int i = 0; i < taskCount; i++) {
    if (tasks[i].state == TaskState::TERMINATED || tasks[i].state == TaskState::UNINITIALIZED) {
      free(tasks[i].stack);
      tasks[i].state = TaskState::UNINITIALIZED;
      reclaimed = i;
      break;
    }
  }

  TCB *tcb = nullptr;

  // increase tasks array only if there are no uninitialized tasks to reclaim
  if (reclaimed == -1) {
    tasks = static_cast<TCB *>(realloc(tasks, sizeof(TCB) * (taskCount + 1)));
    tcb = &tasks[taskCount];
    taskCount++;
  } else {
    tcb = &tasks[reclaimed];
  }

  // Allocate stack
  tcb->stack = static_cast<uint32_t *>(malloc(stackSize * sizeof(uint32_t)));
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
__attribute__((naked)) void Scheduler::taskExit() {
  __disable_irq();
  
  // Get current task before any state changes
  TCB* tcb = currentTask;

  printf("Task %s exited\n", tcb->name);
  tcb->state = TaskState::UNINITIALIZED;
  memset(tcb->name, 0, sizeof(tcb->name));
  free(tcb->stack);
  tcb->stack_pointer = nullptr;
  tcb->stack = nullptr;
  
  __enable_irq();
  switchTasks();
}

// Update next task
void Scheduler::updateNextTask() {
  uint32_t startIndex = taskIndex;
  
  // find next ready task
  do {
    taskIndex = (taskIndex + 1) % (taskCount);
    nextTask = &tasks[taskIndex];
    
    // If we've checked all tasks and found none ready, set to nullptr
    if (taskIndex == startIndex) {
      nextTask = nullptr;
      break;
    }
  } while (nextTask->state != TaskState::READY);
}

// Switch tasks
__attribute__((naked)) void Scheduler::switchTasks() {
  __asm__ __volatile__(
      // save r4-r11 on current task's stack
      "MRS r0, psp\n"                          // r0 = current PSP
      "STMDB r0!, {r4-r11}\n"                  // Push r4-r11, decrement sp
      "LDR r1, =_ZN9Scheduler11currentTaskE\n" // r1 = &Scheduler::currentTask
      "LDR r2, [r1]\n"                         // r2 = currentTask
      "STR r0, [r2]\n"                         // currentTask->stack_pointer = r0

      // Set currentTask to nextTask and update its state
      "LDR r3, =_ZN9Scheduler8nextTaskE\n"     // r3 = &Scheduler::nextTask
      "LDR r4, [r3]\n"                         // r4 = nextTask
      "CMP r4, #0\n"                           // Check if nextTask is nullptr
      "BEQ 1f\n"                               // If nullptr, skip to end
      "STR r4, [r1]\n"                         // currentTask = nextTask
      "MOV r5, #1\n"                           // r5 = TaskState::RUNNING
      "STRB r5, [r4, #8]\n"                    // nextTask->state = RUNNING

      // load r4-r11 from next task's stack
      "LDR r0, [r4]\n"        // r0 = nextTask->stack_pointer
      "LDMIA r0!, {r4-r11}\n" // Pop r4-r11, increment sp
      "MSR psp, r0\n"         // update PSP
      
      "1:\n"                  // Label for branch target

      // return from interrupt
      "BX LR\n"
      :
      :
      : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
}