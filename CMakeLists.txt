cmake_minimum_required(VERSION 3.13)

# Set any variables required for importing libraries
SET(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-Kernel)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

# set the project name
project(rp2040-freertos-template C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--print-memory-usage")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--print-memory-usage")

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.0.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.0.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()

include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add subdirectories
add_subdirectory(src)

# if you have anything in "lib" folder then uncomment below - remember to add a CMakeLists.txt
# file to the "lib" directory
#add_subdirectory(lib)

# show target size info
add_custom_target(
    print-memory-usage ALL
    COMMAND arm-none-eabi-size -G src/${CMAKE_PROJECT_NAME}.elf
    DEPENDS ${CMAKE_BINARY_DIR}/src/${CMAKE_PROJECT_NAME}.elf
    COMMENT "Print target size info"
    DEPENDS ${PROJECT_NAME}
)

# FIXME: Temporary manual operation:
# First, You need to add `set auto-load safe-path .` in your `~/.config/gdb/gdbinit`
# Then you have to run `openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg` etc.
# in another window to start the debugger. Finally, in the build directory, run
# `gdb-multiarch ./src/rp2040-freertos-template.elf` to attach gdb to the debugger.
add_custom_target(
    copy-gdbinit ALL
    COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/.gdbinit ${CMAKE_BINARY_DIR}/
    DEPENDS ${CMAKE_BINARY_DIR}/src/${CMAKE_PROJECT_NAME}.elf
    COMMENT "Copy .gdbinit to build directory"
    DEPENDS ${PROJECT_NAME}
)

# add a firmware flash target
if(${PICO_BOARD} STREQUAL "pico" OR ${PICO_PLATFORM} STREQUAL "rp2040")
        add_custom_target(
            flash
            COMMAND openocd -f interface/cmsis-dap.cfg -c "adapter speed 10000"
                            -f target/rp2040.cfg -s tcl -c "program src/${CMAKE_PROJECT_NAME}.elf verify reset exit"
            DEPENDS ${CMAKE_BINARY_DIR}/src/${CMAKE_PROJECT_NAME}.elf
            COMMENT "Flashing firmware using CMSIS-DAP Debugger..."
            DEPENDS ${PROJECT_NAME}
        )
elseif(${PICO_BOARD} STREQUAL  "pico2" OR ${PICO_PLATFORM} STREQUAL  "rp2350")
        add_custom_target(
            flash
            COMMAND openocd -f interface/cmsis-dap.cfg -c "adapter speed 10000"
                            -f target/rp2350.cfg -s tcl -c "program src/${CMAKE_PROJECT_NAME}.elf verify reset exit"
            DEPENDS ${CMAKE_BINARY_DIR}/src/${CMAKE_PROJECT_NAME}.elf
            COMMENT "Flashing firmware using CMSIS-DAP Debugger..."
            DEPENDS ${PROJECT_NAME}
        )
endif()
