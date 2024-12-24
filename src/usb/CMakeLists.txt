add_library(usb_device)

target_sources(usb_device PRIVATE
    init.c
    usb.c
    usb_config.c
)

target_include_directories(usb_device PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(usb_device PRIVATE
        pico_stdlib
        hardware_irq
        )
