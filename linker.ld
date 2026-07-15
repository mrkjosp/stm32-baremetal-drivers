/* ============================================================================
 * linker.ld — Linker script for STM32F405RG (Cortex-M4)
 *
 * Defines memory regions (Flash/SRAM) and tells the linker where to place
 * each section of the program: vector table, code, initialized data, and
 * zero-initialized data.
 * ========================================================================== */

/* First function to run after reset (defined in startup.c) */
ENTRY(Reset_Handler)

/* Memory map — addresses/sizes from the STM32F405 datasheet */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K  /* non-volatile, code */
  SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 128K   /* volatile, variables */
}

/* Initial stack pointer: top of SRAM (stack grows downward) */
_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
  /* Vector table: must be first in Flash — CPU reads it on reset.
   * KEEP() prevents the linker from discarding it as "unused". */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } > FLASH

  /* Code + read-only constants */
  .text :
  {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
    _etext = .;
  } > FLASH

  /* Initialized variables: stored in Flash, copied to SRAM by startup code.
   * AT> FLASH = load address, > SRAM = run address. */
  _sidata = LOADADDR(.data);
  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > SRAM AT> FLASH

  /* Zero-initialized variables: no Flash storage, cleared by startup code */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > SRAM

  /* Marks end of used RAM (heap would start here) */
  _end = .;
}