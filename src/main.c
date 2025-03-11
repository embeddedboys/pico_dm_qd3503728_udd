// Copyright (c) 2024 embeddedboys developers

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/time.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/stdio_uart.h"

#include "hardware/pll.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "udd.h"
#include "usb.h"
#include "ili9488.h"
#include "ft6236.h"
#include "backlight.h"
#include "tjpgd.h"
#include "JPEGDEC.h"
#include "decoder.h"
#include "test_jpg.h"

#include "debug.h"

struct udd_data g_udd_data = {0};
QueueHandle_t xToFlushQueue = NULL;

static portTASK_FUNCTION(jpg_task_handler, pvParameters)
{
    ili9488_driver_init();
    ft6236_driver_init();

    decoder_drawimg(0, 0, (uint8_t *)screen_480x320, sizeof(screen_480x320));

    busy_wait_ms(10);
    backlight_driver_init();
    backlight_set_level(100);
    pr_info("backlight set to 100%%\n");

    vTaskDelete(NULL);
}

static portTASK_FUNCTION(tp_task_handler, pvParameters)
{
    static struct udd_data *data = &g_udd_data;
    static struct tp_data *tp = &g_udd_data.tp;

    for (;;) {
        if (ft6236_is_pressed()) {
            tp->is_pressed = true;
            tp->x = ft6236_read_x();
            tp->y = ft6236_read_y();
        } else {
            tp->is_pressed = false;
        }

        vTaskDelay(pdMS_TO_TICKS(data->tp_polling_period));
    }

    vTaskDelete(NULL);
}

static portTASK_FUNCTION(usb_task_handler, pvParameters)
{
    usb_device_init();

    while (!usb_is_configured());

    for(;;) tight_loop_contents();
}

int main(void)
{
    /* NOTE: DO NOT MODIFY THIS BLOCK */
#define CPU_SPEED_MHZ (DEFAULT_SYS_CLK_KHZ / 1000)
    if(CPU_SPEED_MHZ > 266 && CPU_SPEED_MHZ <= 360)
        vreg_set_voltage(VREG_VOLTAGE_1_20);
    else if (CPU_SPEED_MHZ > 360 && CPU_SPEED_MHZ <= 396)
        vreg_set_voltage(VREG_VOLTAGE_1_25);
    else if (CPU_SPEED_MHZ > 396)
        vreg_set_voltage(VREG_VOLTAGE_MAX);
    else
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);

    set_sys_clock_khz(CPU_SPEED_MHZ * 1000, true);
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    CPU_SPEED_MHZ * MHZ,
                    CPU_SPEED_MHZ * MHZ);
    stdio_uart_init_full(uart0, 115200, 16, 17);
    // stdio_uart_init_full(uart1, 115200, 23, 24);

    pr_info("\n\n\nPICO DM QD3503728 USB Display Device\n");

    struct udd_data *data = &g_udd_data;

    data->tp_polling_period = FT6236_POLLING_PERIOD;

    xToFlushQueue = xQueueCreate(2, sizeof(struct video_frame));

    TaskHandle_t usb_handler;
    xTaskCreate(usb_task_handler, "usb_task", 256, NULL, (tskIDLE_PRIORITY + 3), &usb_handler);
    vTaskCoreAffinitySet(usb_handler, (1 << 0));

    TaskHandle_t jpg_handler;
    xTaskCreate(jpg_task_handler, "jpg_task", 256, NULL, (tskIDLE_PRIORITY + 2), &jpg_handler);
    vTaskCoreAffinitySet(jpg_handler, (1 << 1));

    TaskHandle_t tp_handler;
    xTaskCreate(tp_task_handler, "tp_task", 256, NULL, (tskIDLE_PRIORITY + 1), &tp_handler);
    vTaskCoreAffinitySet(tp_handler, (1 << 1));

    pr_info("calling freertos scheduler, %lld\n", time_us_64());
    vTaskStartScheduler();
    for(;;);

    return 0;
}
