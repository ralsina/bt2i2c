# Makefile for BT2UART Bridge
# Simplifies building the Pico W firmware

.PHONY: all clean sender receiver flash-sender flash-receiver help

# Default target - build both firmwares
all: sender receiver

# Build sender firmware (Pico W)
sender:
	@echo "Building BT2UART sender firmware..."
	@if [ ! -d build ]; then mkdir build && cd build && cmake ..; fi
	@cd build && make bt2uart
	@echo "✅ Sender firmware built: build/src/bt2uart.uf2"

# Build receiver firmware (Pico)
receiver:
	@echo "Building Pico receiver firmware..."
	@if [ ! -d build ]; then mkdir build && cd build && cmake ..; fi
	@cd build && make pico_receiver
	@echo "✅ Receiver firmware built: build/host/pico_receiver/pico_receiver.uf2"

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf build
	@echo "✅ Build directory cleaned"

# Flash sender to Pico W (copies UF2 to current directory for easy access)
flash-sender: sender
	@echo "📋 Flash bt2uart.uf2 to your Pico W:"
	@echo "1. Hold BOOTSEL button on Pico W"
	@echo "2. Connect USB cable (while holding BOOTSEL)"
	@echo "3. Copy build/src/bt2uart.uf2 to the RPI-RP2 drive"
	@echo ""
	@echo "Location: $(PWD)/build/src/bt2uart.uf2"
	@echo ""
	@ls -lh build/src/bt2uart.uf2

# Flash receiver to Pico
flash-receiver: receiver
	@echo "📋 Flash pico_receiver.uf2 to your Pico:"
	@echo "1. Hold BOOTSEL button on Pico"
	@echo "2. Connect USB cable (while holding BOOTSEL)"
	@echo "3. Copy build/host/pico_receiver/pico_receiver.uf2 to the RPI-RP2 drive"
	@echo ""
	@echo "Location: $(PWD)/build/host/pico_receiver/pico_receiver.uf2"
	@echo ""
	@ls -lh build/host/pico_receiver/pico_receiver.uf2

# Show help
help:
	@echo "BT2UART Bridge - Build Commands"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all              Build both sender and receiver (default)"
	@echo "  sender           Build Pico W sender firmware only"
	@echo "  receiver         Build Pico receiver firmware only"
	@echo "  clean            Remove build directory"
	@echo "  flash-sender     Build sender and show flashing instructions"
	@echo "  flash-receiver   Build receiver and show flashing instructions"
	@echo "  help             Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build everything"
	@echo "  make sender       # Build only Pico W firmware"
	@echo "  make clean        # Clean build artifacts"
	@echo "  make flash-sender # Build and get sender UF2 ready to flash"