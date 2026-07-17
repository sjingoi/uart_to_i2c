cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(uart_to_i2c C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_NO_PICOTOOL 1)
pico_sdk_init()

# Converter binary

add_executable(converter converter_main.c)

pico_enable_stdio_uart(converter 1)
pico_enable_stdio_usb(converter 1)

pico_add_extra_outputs(converter)

target_link_libraries(converter pico_stdlib hardware_uart)

# Driver binary

add_executable(driver driver_main.c)

pico_enable_stdio_uart(driver 1)
pico_enable_stdio_usb(driver 1)

pico_add_extra_outputs(driver)

target_link_libraries(driver pico_stdlib hardware_uart)