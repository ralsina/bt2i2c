# Makefile for BT2I2C Bridge
# Builds the Pico W BLE-to-I2C keyboard bridge firmware

.PHONY: all clean flash help

all:
	@echo "Building BT2I2C sender firmware..."
	@if [ ! -d build ]; then mkdir build && cd build && cmake ..; fi
	@cd build && make bt2i2c
	@echo "Firmware built: build/src/bt2i2c.uf2"

clean:
	@rm -rf build
	@echo "Build directory cleaned"

flash:
	@echo "Flash build/src/bt2i2c.uf2 to your Pico W (hold BOOTSEL)"
	@ls -lh build/src/bt2i2c.uf2

help:
	@echo "BT2I2C Bridge - Build Commands"
	@echo ""
	@echo "  make      Build firmware (build/src/bt2i2c.uf2)"
	@echo "  make clean   Remove build directory"
	@echo "  make flash   Show flashing instructions"
	@echo ""
	@echo "Host: use Arduino/CircuitPython BBQ10KBD library (I2C addr 0x1F)"