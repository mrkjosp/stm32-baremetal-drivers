# Makefile — build the bare-metal STM32 driver project and run it in QEMU
# Targets:
#   make        -> compile + link -> build/firmware.elf
#   make run    -> launch QEMU with the firmware, serial on the terminal
#   make clean  -> remove build artifacts


# --- Toolchain ---
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Target CPU flags (Cortex-M4, no hardware FPU)
CPUFLAGS = -mcpu=cortex-m4 -mthumb

# --- Compiler flags ---
# Warnings on, debug symbols, no optimization, no OS/libc assumptions,don't link standard libs
CFLAGS  = $(CPUFLAGS) -Wall -Wextra -g -O0 -ffreestanding -Iinc
LDFLAGS = $(CPUFLAGS) -nostdlib -T linker.ld -Wl,-Map=build/firmware.map

# --- Sources ---
SRCS = src/startup.c src/main.c src/gpio.c src/uart.c src/ringbuffer.c src/spi.c src/i2c.c
OBJS = $(SRCS:src/%.c=build/%.o)

# --- Default target ---
all: build/firmware.elf

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/firmware.elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@echo "Built build/firmware.elf"

# --- Run in QEMU (netduinoplus2 = an STM32F405 board QEMU supports) ---
# -nographic routes the board's serial port to the terminal.
run: build/firmware.elf
	qemu-system-arm -M netduinoplus2 -nographic -kernel build/firmware.elf

clean:
	rm -rf build

.PHONY: all run clean