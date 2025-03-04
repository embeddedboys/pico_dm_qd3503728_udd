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

#include "ili9488.h"
#include "backlight.h"
#include "tjpgd.h"
#include "test_jpg.h"

#include "debug.h"



uint8_t workspace[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4)));
const uint8_t* array_data = NULL;
uint32_t array_index = 0;
uint32_t array_size  = 0;
int16_t jpeg_x = 0, jpeg_y = 0;
uint start_time, end_time;
static uint frame_count = 0;

unsigned int jd_input(JDEC* jdec, uint8_t* buf, unsigned int len)
{
    memcpy(buf, (const uint8_t *)(array_data + array_index), len);
    array_index += len;

    return len;
}

int jd_output(JDEC* jdec, void* bitmap, JRECT* jrect)
{
    int16_t  x = jrect->left + jpeg_x;
    int16_t  y = jrect->top  + jpeg_y;
    uint16_t w = jrect->right  - jrect->left;
    uint16_t h = jrect->bottom - jrect->top;

    // printf("%d, %d, %d, %d\n", jrect->top, jrect->left, jrect->bottom, jrect->right);
    // printf("%d, %d, %d, %d\n", x, y, w, h);

    ili9488_video_flush(x, y, x + w, y + h, (uint16_t *)bitmap, (w + 1) * (h + 1));
}

JRESULT jd_getsize(uint16_t *w, uint16_t *h, const uint8_t jpeg_data[], uint32_t  data_size)
{
    JDEC jdec;
    JRESULT jresult = JDR_OK;

    array_index = 0;
    array_data  = jpeg_data;
    array_size  = data_size;

    jresult =  jd_prepare(&jdec, jd_input, workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (jresult == JDR_OK) {
        *w = jdec.width;
        *h = jdec.height;
    }

    return jresult;
}

JRESULT jd_drawjpg(int32_t x, int32_t y, const uint8_t jpeg_data[], uint32_t  data_size)
{
    JDEC jdec;
    JRESULT jresult = JDR_OK;

    array_index = 0;
    array_data  = jpeg_data;
    array_size  = data_size;

    jpeg_x = x;
    jpeg_y = y;

    jdec.swap = false;

    jresult = jd_prepare(&jdec, jd_input, workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (jresult == JDR_OK) {
        jresult = jd_decomp(&jdec, jd_output, 0);
    }

    return jresult;
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

    printf("\n\n\nPICO DM QD3503728 UDD Tjpgd Perf test\n");

    ili9488_driver_init();
    backlight_driver_init();
    backlight_set_level(100);
    printf("backlight set to 100%%\n");

    printf("Running *Tjpgd* decode test...\n");
    uint sum_time = 0;
    for (;;) {
        start_time = time_us_32();
        jd_drawjpg(0, 0, screen_480x320, sizeof(screen_480x320));
        end_time = time_us_32();

        sum_time += end_time - start_time;

        frame_count++;
        printf(".");

        if (frame_count == 50) {
            break;
        }
    }
    printf("\n");
    printf("| CPU Speed: %d MHz ", CPU_SPEED_MHZ);
    printf("| *Tjpgd* decode avg. time: %d us | FPS : %d |\n",
        sum_time / frame_count, 1000000 / (sum_time / frame_count));

    return 0;
}
