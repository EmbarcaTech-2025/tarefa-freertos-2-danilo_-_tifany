# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Set any variables required for importing libraries
SET(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS)
message("FreeRTOS Kernel located in ${FREERTOS_PATH}")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(embarcatech-tarefa-freertos-2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(embarcatech-tarefa-freertos-2
        src/main.c
        lib/ssd1306/ssd1306.c
        src/game.c
        src/tasks/effects_task.c
        src/drivers/hardware_init.c
        src/tasks/oled_task.c
        src/tasks/player_task.c
        src/tasks/game_logic.c
        src/tasks/alien_task.c
        src/tasks/bullet_task.c
        src/drivers/rgb.c
        src/tasks/pause_task.c
        src/drivers/buzzer.c
        )

pico_set_program_name(embarcatech-tarefa-freertos-2 "embarcatech-tarefa-freertos-2")
pico_set_program_version(embarcatech-tarefa-freertos-2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(embarcatech-tarefa-freertos-2 1)
pico_enable_stdio_usb(embarcatech-tarefa-freertos-2 1)

# Add the standard library to the build
target_link_libraries(embarcatech-tarefa-freertos-2
        pico_stdlib
        pico_rand
        FreeRTOS-Kernel-Heap4
        hardware_i2c
        hardware_adc
        hardware_pwm)

# Add the standard include files to the build
target_include_directories(embarcatech-tarefa-freertos-2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306/include
)

# Add any user requested libraries
target_link_libraries(embarcatech-tarefa-freertos-2 
        
        )

pico_add_extra_outputs(embarcatech-tarefa-freertos-2)