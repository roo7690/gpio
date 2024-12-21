cmake_minimum_required(VERSION 3.13)
include($ENV{LIB_PATH}/pico-sdk/external/pico_sdk_import.cmake)

project(led)
pico_sdk_init()
add_executable(led
  main.c
)
include_directories($ENV{WFM}/pico/abstract)
target_link_libraries(led pico_stdlib)
pico_add_extra_outputs(led)