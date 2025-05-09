TOOLCHAIN := arm-none-eabi
USER := $(shell whoami)

# Compiler flags
CFLAGS := -mcpu=cortex-m7 -mthumb -fPIC -nostdlib -g -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti
CFLAGS += -DSTM32H723xx
CFLAGS += -DUSE_HAL_DRIVER
CFLAGS += -DUSE_FULL_LL_DRIVER

# These have to match the build flags in platformio.ini or shit goes haywire
CFLAGS += -DENABLE_FATFS=1
CFLAGS += -DENABLE_MICROSD=1
CFLAGS += -DENABLE_LCD=1
CFLAGS += -DENABLE_ERROR_STRINGS=1
CFLAGS += -DENABLE_ALLOCATION_TRACKER=0

# Include paths
CFLAGS += -I/home/$(USER)/.platformio/packages/framework-stm32cubeh7/Drivers/STM32H7xx_HAL_Driver/Inc
CFLAGS += -I/home/$(USER)/.platformio/packages/framework-stm32cubeh7/Drivers/CMSIS/Device/ST/STM32H7xx/Include
CFLAGS += -I/home/$(USER)/.platformio/packages/framework-stm32cubeh7/Drivers/CMSIS/Include
CFLAGS += -I../src
CFLAGS += -I../src/system
CFLAGS += -I../src/peripherals
CFLAGS += -I../src/middleware/FatFs

# Linker flags
LDFLAGS := -Tdynamic_task.ld -Wl,--gc-sections -Wl,--print-memory-usage -Wl,--no-warn-rwx-segments

# Default target
all: dynamic_task.bin

# Extract symbols from firmware
firmware.sym: ../.pio/build/stm32h723weact/firmware.elf
	$(TOOLCHAIN)-nm -g --defined-only $< > $@

# Generate symbol definitions
symbols.ld: firmware.sym
	@echo "/* Auto-generated symbol definitions */" > $@
	@while read -r line; do \
		addr=$$(echo "$$line" | awk '{print $$1}'); \
		type=$$(echo "$$line" | awk '{print $$2}'); \
		name=$$(echo "$$line" | awk '{print $$3}'); \
		if [[ "$$type" == "T" || "$$type" == "D" || "$$type" == "B" ]]; then \
			echo "PROVIDE($$name = 0x$$addr);" >> $@; \
		fi \
	done < $<

# Compile source
dynamic_task.o: dynamic_task.cpp
	$(TOOLCHAIN)-g++ $(CFLAGS) -c $< -o $@

# Link object file
dynamic_task.elf: dynamic_task.o symbols.ld
	$(TOOLCHAIN)-g++ $(CFLAGS) $(LDFLAGS) $< -o $@

# Create binary
dynamic_task.bin: dynamic_task.elf
	$(TOOLCHAIN)-objcopy -O binary --only-section=.text --only-section=.data --only-section=.rodata $< $@
	@echo "Binary contents:"
	@hexdump -C $@ | head -n 5
	@echo "Binary size: $$(stat -c%s $@) bytes"

# Clean up
clean:
	rm -f dynamic_task.o symbols.ld firmware.sym dynamic_task.elf dynamic_task.bin

.PHONY: all clean upload

# Upload binary to SD card
upload: dynamic_task.bin
	@echo "Mounting SD card..."
	@sudo mount /dev/sdc1 /mnt
	@echo "Copying binary to SD card..."
	@sudo cp dynamic_task.bin /mnt/
	@echo "Unmounting SD card..."
	@sudo umount /mnt
	@echo "Upload complete!" 