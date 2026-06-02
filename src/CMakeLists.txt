add_executable(bt2i2c_ble
    main.c
    bt_keyboard.c
    display.cpp
    reg.c
    i2c_slave.c
    fifo.c
)

# Set C++ standard for LovyanGFX compatibility
set_target_properties(bt2i2c_ble PROPERTIES
    CXX_STANDARD 17
    C_STANDARD 11
)

target_include_directories(bt2i2c_ble PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/btstack/src
    ${PICO_SDK_PATH}/lib/btstack/platform/embedded
    ${PICO_SDK_PATH}/lib/btstack
    ${PICO_SDK_PATH}/lib/btstack/3rd-party/bluedroid/encoder/include
    ${PICO_SDK_PATH}/lib/btstack/3rd-party/bluedroid/decoder/include
)

target_link_libraries(bt2i2c_ble
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_cyw43
    pico_btstack_ble
    hardware_i2c
    hardware_spi
    hardware_gpio
    hardware_pwm
    LovyanGFX
)

pico_add_extra_outputs(bt2i2c_ble)
pico_enable_stdio_uart(bt2i2c_ble 1)
pico_enable_stdio_usb(bt2i2c_ble 1)
