cmake_minimum_required(VERSION 3.20.0)
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../common/boards/m5stack_fire_esp32_procpu.overlay)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(task3_state_machine)
target_sources(app PRIVATE src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/display_direct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/font16.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../common)
