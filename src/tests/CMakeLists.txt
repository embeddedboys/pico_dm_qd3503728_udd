# user define common source files
file(GLOB_RECURSE COMMON_SOURCES
    ../ili9488.c
    ../ft6236.c
    ../i2c_tools.c
    ../backlight.c
)

# TJPGD Performance Test
add_executable(tjpgd_perf_test ${COMMON_SOURCES} tjpgd_perf.c)
target_link_libraries(tjpgd_perf_test
    pico_stdlib
    pico_multicore
    FreeRTOS-Kernel-Heap3
    pico_bootsel_via_double_reset
    hardware_pwm
    hardware_i2c
    pio_i80
    tjpg_decoder
)

target_compile_definitions(tjpgd_perf_test PUBLIC DEFAULT_SYS_CLK_KHZ=${SYS_CLK_KHZ})
target_compile_definitions(tjpgd_perf_test PUBLIC DEFAULT_PERI_CLK_KHZ=${PERI_CLK_KHZ})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_DB_BASE=${LCD_PIN_DB_BASE})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_DB_COUNT=${LCD_PIN_DB_COUNT})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_CS=${LCD_PIN_CS})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_WR=${LCD_PIN_WR})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_RS=${LCD_PIN_RS})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_RST=${LCD_PIN_RST})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_PIN_BL=${LCD_PIN_BL})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_ROTATION=${LCD_ROTATION})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_HOR_RES=${LCD_HOR_RES})
target_compile_definitions(tjpgd_perf_test PUBLIC LCD_VER_RES=${LCD_VER_RES})
target_compile_definitions(tjpgd_perf_test PUBLIC DISP_OVER_PIO=${DISP_OVER_PIO})

# JPEGDEC Performance Test
add_executable(jpegdec_perf_test ${COMMON_SOURCES} jpegdec_perf.c)
target_link_libraries(jpegdec_perf_test
    pico_stdlib
    pico_multicore
    FreeRTOS-Kernel-Heap3
    pico_bootsel_via_double_reset
    hardware_pwm
    hardware_i2c
    pio_i80
    jpegdec
)

target_compile_definitions(jpegdec_perf_test PUBLIC
    DEFAULT_SYS_CLK_KHZ=${SYS_CLK_KHZ}
    DEFAULT_PERI_CLK_KHZ=${PERI_CLK_KHZ}
    LCD_PIN_DB_BASE=${LCD_PIN_DB_BASE}
    LCD_PIN_DB_COUNT=${LCD_PIN_DB_COUNT}
    LCD_PIN_CS=${LCD_PIN_CS}
    LCD_PIN_WR=${LCD_PIN_WR}
    LCD_PIN_RS=${LCD_PIN_RS}
    LCD_PIN_RST=${LCD_PIN_RST}
    LCD_PIN_BL=${LCD_PIN_BL}
    LCD_ROTATION=${LCD_ROTATION}
    LCD_HOR_RES=${LCD_HOR_RES}
    LCD_VER_RES=${LCD_VER_RES}
    DISP_OVER_PIO=${DISP_OVER_PIO}
)