# Bare-Metal STM32 Peripheral Drivers (register-level, no HAL)

Bare-metal GPIO, UART, SPI, and I2C drivers for an STM32F405 (ARM Cortex-M4),
written directly from the STMicroelectronics reference manual (RM0090) — no HAL, no CubeMX. 
Built with `arm-none-eabi-gcc` + `make` and run under QEMU. 
The goal is to demonstrate the silicon-bring-up / driver work that involve:
memory-mapped I/O, the clock tree, interrupt handling, and serial protocols, all at the register level.

## What's here

| Module | What it does | Key concepts |
|---|---|---|
| `linker.ld` | Memory map: FLASH/SRAM regions, section placement, stack top | LMA vs VMA, `.data`/`.bss`, stack |
| `src/startup.c` | Vector table + reset handler (runs before `main`) | what happens before `main()`, `.data` copy, `.bss` zero |
| `src/gpio.c` | GPIO driver: mode, alternate-function, read/write | memory-mapped I/O, read-modify-write, clock-enable gotcha |
| `src/uart.c` | USART driver: polled TX, **interrupt-driven RX + ring buffer** | ISRs, NVIC, producer/consumer, `volatile` |
| `src/ringbuffer.c` | Lock-free single-producer/single-consumer byte buffer | lock-free SPSC, power-of-two masking |
| `src/spi.c` | SPI master, full-duplex transfer | CPOL/CPHA modes, full-duplex shift |
| `src/i2c.c` | I2C master register read | start/stop/ACK, repeated start |

## Build and run

```bash
make            # compile + link -> build/firmware.elf
make run        # launch in QEMU; serial console on your terminal
make clean
```

In `make run`, the boot banner prints over UART and anything typed is echoed
back exercising the full interrupt-driven receive path (UART ISR → ring buffer → main loop). 
Press `Ctrl-A` then `X` to quit QEMU.

## How this was validated

- **GPIO and UART**: executed end-to-end under QEMU (`netduinoplus2` machine).
  The UART echo test confirms the ISR → ring-buffer → main-loop path works
  without dropping bytes.
- **SPI and I2C**: **logic-validated against the reference manual, not executed end-to-end.** 
  QEMU's STM32 SPI/I2C peripheral models are incomplete, and more importantly there is no simulated device 
  on the bus for a transaction to talk to. 
  The driver logic (register sequences, flag waits, the I2C repeated-start read) follows RM0090.
  To execute them end-to-end I would run them in **Renode** against a simulated sensor, 
  or on real STM32F4 hardware (which additionally needs external SDA/SCL pull-ups for I2C).


## Ring buffer behavior

The UART RX ring buffer holds 127 usable bytes out of 128 slots — one slot is
kept empty so `head == tail` unambiguously means "empty" rather than being
ambiguous with "full." If a byte arrives while the buffer is full, it is
dropped rather than overwriting unread data or blocking the ISR.

## Note on QEMU pin/port choices vs real hardware

QEMU's `netduinoplus2` routes its serial console to **USART1 (PA9/PA10)**, so the
UART driver targets USART1. On a typical STM32F4 dev board you'd use **USART2
(PA2/PA3)** — the ST-Link virtual COM port. The driver logic is identical; only
the base address, the clock-enable bit (APB2 vs APB1), the pins, and the IRQ
number differ. The same code runs on real silicon after that one-line retarget.

## What would differ on real silicon

- **Clocking**: this uses a fixed 16 MHz reference for baud/timing. On hardware
  you configure the PLL and compute baud/CCR from the actual bus clock.
- **I2C**: needs external pull-up resistors on SDA/SCL; timing (CCR/TRISE)
  depends on the real bus clock and capacitance.
- **Signal integrity**: rise times, noise, and electrical loading don't exist in
  emulation but matter on a board (and are debugged with a scope/logic analyzer).

## Toolchain

`arm-none-eabi-gcc`, `make`, `qemu-system-arm`. A `.devcontainer` is included so
the project builds out-of-the-box in GitHub Codespaces with no local setup.