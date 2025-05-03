.syntax unified
.cpu cortex-m7
.thumb

.global _ZN9Scheduler11switchTasksEv
.type _ZN9Scheduler11switchTasksEv, %function

.global _ZN9Scheduler5startEv
.type _ZN9Scheduler5startEv, %function

.global _ZN9Scheduler5yieldEv
.type _ZN9Scheduler5yieldEv, %function

_ZN9Scheduler5yieldEv:
  // check if scheduler is active
  LDR r0, =_ZN9Scheduler6activeE
  LDR r0, [r0]
  CMP r0, #0
  BEQ yield_exit

  // set PendSV bit
  LDR r0, =0xE000ED04
  LDR r1, =0x10000000
  STR r1, [r0]

yield_exit:
  BX LR

_ZN9Scheduler5startEv:
  // Check if tasks is nullptr
  LDR r0, =_ZN9Scheduler5tasksE
  LDR r0, [r0]
  CMP r0, #0
  BEQ start_exit

  // Set active to true
  LDR r1, =_ZN9Scheduler6activeE
  MOV r2, #1
  STR r2, [r1]

  // Set currentTask to &tasks[0]
  LDR r1, =_ZN9Scheduler11currentTaskE
  STR r0, [r1]

  // Set currentTask->state to RUNNING (offset 8 for state)
  MOV r2, #2  // RUNNING state
  STR r2, [r0, #8]

  // Set taskIndex to 0
  LDR r1, =_ZN9Scheduler9taskIndexE
  MOV r2, #0
  STR r2, [r1]

  // Load stack pointer and set up PSP
  LDR r0, [r0]  // Load stackPointer from currentTask
  MSR psp, r0   // Set PSP to stackPointer

  // Set CONTROL register to use PSP in thread mode, privileged
  MOV r0, #2
  MSR CONTROL, r0

  // Instruction Synchronization Barrier
  ISB

  // Pop all registers and jump to task
  POP {r4-r11}  // Pop callee-saved registers
  POP {r0-r3}   // Pop initial registers
  POP {r12}     // Pop r12
  POP {lr}      // Pop LR
  POP {pc}      // Jump to task

start_exit:
  BX LR

_ZN9Scheduler11switchTasksEv:
  // disable interrupts
  CPSID I

  // check if currentTask is nullptr
  LDR r1, =_ZN9Scheduler11currentTaskE
  LDR r2, [r1]
  CMP r2, #0
  BEQ skip_context_save

  // save r4-r11 on current task's stack
  MRS r0, psp
  STMDB r0!, {r4-r11}
  STR r0, [r2]

  // Set currentTask->state to READY
  MOV r4, #1
  STR r4, [r2, #8]

skip_context_save:
  // Set currentTask to nextTask
  LDR r3, =_ZN9Scheduler8nextTaskE
  LDR r4, [r3]
  STR r4, [r1]

  // Set nextTask->state to RUNNING
  MOV r5, #2
  STR r5, [r4, #8]

  // load r4-r11 from next task's stack
  LDR r0, [r4]
  LDMIA r0!, {r4-r11}
  MSR psp, r0

  // Instruction syncronization barrier
  ISB

  // restore interrupts
  CPSIE I
  
  // return from interrupt
  BX LR 