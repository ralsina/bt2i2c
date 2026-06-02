# Makefile for BT2I2C Bridge
# Builds the Pico W BLE-to-I2C keyboard bridge firmware

.PHONY: all clean flash help

all:
	@echo "Building BT2I2C sender firmware..."
	@if [ ! -d build ]; then mkdir build && cd build && cmake ..; fi
	@cd build && make bt2i2c_ble
	@echo "Firmware built: build/src/bt2i2c_ble.uf2"

clean:
	@rm -rf build
	@echo "Build directory cleaned"

flash:
	@echo "Flash build/src/bt2i2c_ble.uf2 to your Pico W (hold BOOTSEL)"
	@ls -lh build/src/bt2i2c_ble.uf2
	@if [ -d /run/media/$(USER)/RPI-RP2 ]; then \
		cp build/src/bt2i2c_ble.uf2 /run/media/$(USER)/RPI-RP2/ && \
		echo "Firmware copied successfully!"; \
	else \
		echo "RPI-RP2 not found. Hold BOOTSEL and plug in Pico W."; \
	fi

help:
	@echo "BT2I2C Bridge - Build Commands"
	@echo ""
	@echo "  make      Build firmware (build/src/bt2i2c_ble.uf2)"
	@echo "  make clean   Remove build directory"
	@echo "  make flash   Show flashing instructions"
	@echo ""
	@echo "Host: use Arduino/CircuitPython BBQ10KBD library (I2C addr 0x1F)"