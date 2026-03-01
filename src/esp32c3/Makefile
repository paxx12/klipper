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
src-$(CONFIG_WANT_ADC) += esp32c3/adc.c
src-$(CONFIG_WANT_SPI) += esp32c3/spi.c
src-$(CONFIG_USB) += esp32c3/chipid.c
src-$(CONFIG_ESP32C3_SERIAL_UART0_IRQ) += esp32c3/serial.c generic/serial_irq.c

# Build output binary using esptool
$(OUT)klipper.bin: $(OUT)klipper.elf
	@echo "  Creating binary $@"
	$(Q)esptool.py --chip esp32c3 elf2image --output $@ $<

target-y += $(OUT)klipper.bin

# Linker rules
CFLAGS_klipper.elf += -nostdlib -lgcc
CFLAGS_klipper.elf += -T $(OUT)src/esp32c3/esp32c3_link.ld
$(OUT)klipper.elf: $(OUT)src/esp32c3/esp32c3_link.ld

FLASH_BAUD ?= 460800
FLASH_OFFSET := $(shell printf '0x%x' $(CONFIG_FLASH_APPLICATION_ADDRESS))

# Flash via serial using esptool (enter download mode via DTR/RTS auto-reset)
flash: $(OUT)klipper.bin
	@echo "  Flashing $< to $(FLASH_DEVICE) at offset $(FLASH_OFFSET)"
	$(Q)esptool.py --chip esp32c3 -p "$(FLASH_DEVICE)" -b $(FLASH_BAUD) \
		--before default_reset --after hard_reset \
		write_flash $(FLASH_OFFSET) $<
