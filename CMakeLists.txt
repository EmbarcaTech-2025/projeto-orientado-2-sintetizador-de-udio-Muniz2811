# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)


set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../pico-sdk)

include(pico_sdk_import.cmake)


project(audio_synthesizer C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(audio_synthesizer
    src/audio_synthesizer.c
    src/ssd1306.c
    src/ssd1306_i2c.c
)

target_link_libraries(audio_synthesizer
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_dma
    hardware_i2c
)

pico_enable_stdio_usb(audio_synthesizer 1)
pico_enable_stdio_uart(audio_synthesizer 0)

pico_add_extra_outputs(audio_synthesizer)
