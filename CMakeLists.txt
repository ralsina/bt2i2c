cmake_minimum_required(VERSION 3.13)

# Set board to Pico W for BLE support
set(PICO_BOARD pico_w)

# Initialize the SDK - use local copy from terminado project
set(PICO_SDK_PATH "/home/ralsina/code/terminado/bt2i2c/3rdparty/pico-sdk")
include(pico_sdk_import.cmake)

# Build the project
pico_sdk_init()

# Define the project
project(bt2i2c_ble C CXX ASM)

# Add subdirectory
add_subdirectory(src)
