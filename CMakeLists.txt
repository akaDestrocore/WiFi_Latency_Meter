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

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(WiFi_Latency_Meter C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(WiFi_Latency_Meter 
        src/main.c
        src/sensors.c
)

pico_set_program_name(WiFi_Latency_Meter "WiFi_Latency_Meter")
pico_set_program_version(WiFi_Latency_Meter "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(WiFi_Latency_Meter 0)
pico_enable_stdio_usb(WiFi_Latency_Meter 1)

# Add the standard library to the build
target_link_libraries(WiFi_Latency_Meter
        pico_stdlib
        hardware_adc)

# Add the standard include files to the build
target_include_directories(WiFi_Latency_Meter PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

# Add any user requested libraries
target_link_libraries(WiFi_Latency_Meter 
        pico_cyw43_arch_lwip_threadsafe_background
)

pico_add_extra_outputs(WiFi_Latency_Meter)

add_custom_target(copy-uf2
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2
        /mnt/rpi-rp2/
    DEPENDS ${PROJECT_NAME}.uf2
    COMMENT "Copying UF2 to /mnt/rpi-rp2"
)
