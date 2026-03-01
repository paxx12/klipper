# Additional ESP32-C3 build rules

# Setup the toolchain (Espressif RISC-V toolchain)
CROSS_PREFIX=riscv32-esp-elf-

dirs-y += src/esp32c3 src/generic

CFLAGS += -march=rv32imc_zicsr -mabi=ilp32
CFLAGS += -ffreestanding -fno-builtin

# Add source files
src-y += esp32c3/main.c esp32c3/gpio.c
src-y += esp32c3/timer.c esp32c3/watchdog.c
src-y += generic/crc16_ccitt.c generic/timer_irq.c

# Build output binary using esptool
$(OUT)klipper.bin: $(OUT)klipper.elf
	@echo "  Creating binary $@"
	$(Q)esptool.py --chip esp32c3 elf2image --output $@ $<

target-y += $(OUT)klipper.bin

# Linker rules
CFLAGS_klipper.elf += -nostdlib -lgcc
CFLAGS_klipper.elf += -T $(OUT)src/esp32c3/esp32c3_link.ld
$(OUT)klipper.elf: $(OUT)src/esp32c3/esp32c3_link.ld

# Flash rules
flash: $(OUT)klipper.bin
	@echo "  Flashing $< to $(FLASH_DEVICE)"
	$(Q)esptool.py --chip esp32c3 -p "$(FLASH_DEVICE)" -b 460800 \
		write_flash 0x0 $<
