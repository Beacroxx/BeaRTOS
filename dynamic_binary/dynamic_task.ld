/* Linker script for dynamic tasks */
/* Independent dynamic binary using syscalls */

MEMORY
{
  /* Match the MPU configuration from main.cpp */
  CODE (rx) : ORIGIN = 0x24080000, LENGTH = 256K
  RAM (rwx) : ORIGIN = 0x240C0000, LENGTH = 64K  /* Stack space */
}

SECTIONS
{
  .init : {
    KEEP(*(.init))
    . = ALIGN(4);
  } > CODE

  .text : {
    KEEP(*(.text.entry))  /* Entry point must be first */
    *(.text*)            /* Code */
    *(.rodata*)          /* Read-only data */
    *(.glue_7)           /* ARM/Thumb glue */
    *(.glue_7t)          /* Thumb/ARM glue */
    *(.eh_frame)         /* Exception handling */
    . = ALIGN(4);
  } > CODE

  .data : {
    *(.data*)            /* Initialized data */
    . = ALIGN(4);
  } > RAM AT > CODE

  .bss : {
    *(.bss*)             /* Uninitialized data */
    *(COMMON)            /* Common symbols */
    . = ALIGN(4);
  } > RAM

  /* Stack at the end of RAM */
  .stack (NOLOAD) : {
    . = ALIGN(8);
    . = . + 0x1000;      /* 4KB stack */
    . = ALIGN(8);
    _stack_top = .;
  } > RAM

  /* Symbols for the code loader */
  _code_start = ORIGIN(CODE);
  _code_size = LENGTH(CODE);
  _ram_start = ORIGIN(RAM);
  _ram_size = LENGTH(RAM);

  /DISCARD/ : {
    *(.ARM.exidx*)
    *(.ARM.extab*)
  }
} 