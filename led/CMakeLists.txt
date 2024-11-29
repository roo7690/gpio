cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led)
pico_sdk_init()
add_executable(led
  main.c $ENV{wfm}/abstract/gpio/gpio_pico.c
)
include_directories($ENV{wfm}/abstract/gpio)
target_link_libraries(led pico_stdlib)
pico_add_extra_outputs(led)