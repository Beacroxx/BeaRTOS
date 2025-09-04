TOOLCHAIN := arm-none-eabi
USER := $(shell whoami)

# Compiler flags
CFLAGS := -mcpu=cortex-m7 -mthumb -fPIC -O3 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti

# Linker flags
LDFLAGS := -Tdynamic_task.ld -Wl,--gc-sections -Wl,--print-memory-usage -Wl,--no-warn-rwx-segments -s

# Default target
all: dynamic_task.bin

# No symbol extraction needed - using syscalls for firmware independence

# Compile source
dynamic_task.o: dynamic_task.cpp
	rm -f dynamic_task.o
	$(TOOLCHAIN)-g++ $(CFLAGS) -c $< -o $@

# Link object file
dynamic_task.elf: dynamic_task.o
	$(TOOLCHAIN)-g++ $(CFLAGS) $(LDFLAGS) $< -o $@

# Create binary
dynamic_task.bin: dynamic_task.elf
	$(TOOLCHAIN)-objcopy -O binary --only-section=.text --only-section=.data --only-section=.rodata $< $@
	@echo "Binary contents:"
	@hexdump -C $@ | head -n 32
	@echo "Binary size: $$(stat -c%s $@) bytes"

# Clean up
clean:
	rm -f dynamic_task.o dynamic_task.elf dynamic_task.bin

.PHONY: all clean upload

# Upload binary to SD card
upload: dynamic_task.bin
	@echo "Mounting SD card..."
	@sudo mount /dev/sdd1 /mnt
	@echo "Copying binary to SD card..."
	@sudo cp dynamic_task.bin /mnt/
	@echo "Unmounting SD card..."
	@sudo umount /mnt
	@echo "Upload complete!" 