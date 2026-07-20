cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(uart_to_i2c C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_NO_PICOTOOL 1)
pico_sdk_init()

# master binary

add_executable(master master_main.c)

pico_enable_stdio_uart(master 1)
pico_enable_stdio_usb(master 1)

pico_add_extra_outputs(master)

target_link_libraries(master pico_stdlib hardware_uart hardware_i2c hardware_pwm)

# slave binary

add_executable(slave slave_main.c)

pico_enable_stdio_uart(slave 1)
pico_enable_stdio_usb(slave 1)

pico_add_extra_outputs(slave)

target_link_libraries(slave pico_stdlib hardware_uart hardware_i2c pico_i2c_slave hardware_pwm)