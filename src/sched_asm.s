.syntax unified
.cpu cortex-m7
.thumb

.global _ZN5SCHED11switchTasksEv
.type _ZN5SCHED11switchTasksEv, %function

.global _ZN5SCHED5startEv
.type _ZN5SCHED5startEv, %function

_ZN5SCHED5startEv:
  LDR r0, =_ZN5SCHED5tasksE
  LDR r0, [r0]
  CMP r0, #0
  BEQ start_exit

  LDR r1, =_ZN5SCHED6activeE
  MOV r2, #1
  STR r2, [r1]

  LDR r1, =_ZN5SCHED11currentTaskE
  STR r0, [r1]

  MOV r2, #2
  STR r2, [r0, #8]

  LDR r1, =_ZN5SCHED9taskIndexE
  MOV r2, #0
  STR r2, [r1]

  LDR r0, [r0]
  MSR psp, r0

  MOV r0, #2
  MSR CONTROL, r0

  ISB

  POP {r4-r11}
  POP {r0-r3}
  POP {r12}
  POP {lr}
  POP {pc}

start_exit:
  BX LR

_ZN5SCHED11switchTasksEv:
  CPSID I

  LDR r1, =_ZN5SCHED11currentTaskE
  LDR r2, [r1]
  CMP r2, #0
  BEQ skip_context_save

  MRS r0, psp
  STMDB r0!, {r4-r11}
  STR r0, [r2]

  MOV r4, #1
  STR r4, [r2, #8]

skip_context_save:
  LDR r3, =_ZN5SCHED8nextTaskE
  LDR r4, [r3]
  STR r4, [r1]

  MOV r5, #2
  STR r5, [r4, #8]

  LDR r0, [r4]
  LDMIA r0!, {r4-r11}
  MSR psp, r0

  ISB

  CPSIE I
  
  BX LR 
