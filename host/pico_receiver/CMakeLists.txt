cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../3rdparty/pico-sdk")
include(../../3rdparty/pico-sdk/external/pico_sdk_import.cmake)

project(pico_receiver)

pico_sdk_init()

add_executable(pico_receiver
    receiver.c
)

target_link_libraries(pico_receiver
    pico_stdlib
    hardware_uart
)

pico_enable_stdio_uart(pico_receiver 0)
pico_enable_stdio_usb(pico_receiver 1)
pico_add_extra_outputs(pico_receiver)
