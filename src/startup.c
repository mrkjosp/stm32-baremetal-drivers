/* startup.c  —  Reset handler + interrupt vector table (STM32F405, Cortex-M4)
 *
 * On reset the CPU:
 *   1. Loads the initial stack pointer from address 0x08000000 (vector[0]).
 *   2. Loads the reset handler address from 0x08000004 (vector[1]) and jumps to it.
 * Our Reset_Handler then prepares the C runtime environment (copies initialized
 * data into RAM, zeroes uninitialized data) and finally calls main().
 * Without this, global variables would have garbage values and main() couldn't run. */

#include <stdint.h>

/* Symbols defined by the LINKER SCRIPT (linker.ld). They are addresses, not
 * variables — we take their address (&) to get the boundary locations. */
extern uint32_t _sidata;   /* flash: source of .data initial values */
extern uint32_t _sdata;    /* ram:   start of .data */
extern uint32_t _edata;    /* ram:   end of .data   */
extern uint32_t _sbss;     /* ram:   start of .bss  */
extern uint32_t _ebss;     /* ram:   end of .bss    */
extern uint32_t _estack;   /* top of stack          */

int main(void);                 /* forward declaration */
void Reset_Handler(void);
void Default_Handler(void);

/* The USART1 interrupt handler is defined in uart.c. We declare it "weak"
 * here so the linker uses the real one if present, else Default_Handler.
 * Same idea for the SysTick handler (unused for now). */
void USART1_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* ---- The vector table ----
 * Placed by the linker into the .isr_vector section, which the linker script
 * forces to the very start of flash. Each entry is an address the CPU jumps to
 * when the corresponding event occurs. Entry 0 is special: it's not a handler,
 * it's the initial stack pointer value.
 *
 * For STM32F405, USART1's interrupt is IRQ number 37, so its slot in the table
 * is at index (16 core exceptions + 37). We fill the table up to that slot.
 * The 16 leading entries are the ARM core exceptions (reset, NMI, faults...). */
__attribute__((section(".isr_vector")))
const void *vector_table[] = {
    (void *)&_estack,        /*  0: initial stack pointer            */
    Reset_Handler,           /*  1: reset                            */
    Default_Handler,         /*  2: NMI                              */
    Default_Handler,         /*  3: HardFault                        */
    Default_Handler,         /*  4: MemManage                        */
    Default_Handler,         /*  5: BusFault                         */
    Default_Handler,         /*  6: UsageFault                       */
    0, 0, 0, 0,              /*  7-10: reserved                      */
    Default_Handler,         /* 11: SVCall                           */
    Default_Handler,         /* 12: Debug Monitor                    */
    0,                       /* 13: reserved                         */
    Default_Handler,         /* 14: PendSV                           */
    SysTick_Handler,         /* 15: SysTick                          */
    /* ---- External interrupts (IRQ0 onward) start here, index 16 ---- */
    [16 + 37] = USART1_IRQHandler,  /* IRQ37 = USART1 (designated initializer) */
};

/* ---- The reset handler: prepares C runtime, then calls main ---- */
void Reset_Handler(void)
{
    /* 1. Copy initialized data (.data) from its flash storage into RAM. */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 2. Zero the .bss section (uninitialized globals must start at 0). */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    /* 3. Hand control to the application. */
    main();

    /* main() should never return; */
    while (1) { }
}

/* Catch-all for any unhandled interrupt. */
void Default_Handler(void)
{
    while (1) { }
}