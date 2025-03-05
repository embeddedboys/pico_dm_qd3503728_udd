// Copyright (c) 2024 embeddedboys developers
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
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
#include "JPEGDEC.h"
#include "test_jpg.h"

#include "debug.h"

JPEGIMAGE jpeg;

uint start_time, end_time;
uint frame_count;

void draw_mcus(JPEGDRAW *pDraw)
{
	int iCount = pDraw->iWidth * pDraw->iHeight;
	int xs = pDraw->x;
	int ys = pDraw->y;
	int xe = pDraw->x + pDraw->iWidth - 1;
	int ye = pDraw->y + pDraw->iHeight - 1;

	ili9488_video_flush(xs, ys, xe, ye, pDraw->pPixels, iCount);
}

static void jpegenc_drawjpg(int x, int y, uint8_t *jpeg_data, uint32_t jpeg_size)
{
	int ret;

	JPEG_setPixelType(&jpeg, RGB565_LITTLE_ENDIAN);

	ret = JPEG_openRAM(&jpeg, jpeg_data, jpeg_size, draw_mcus);
	if (ret) {
		// printf("Successfully opened JPEG image\n");
		// printf("Image size: %d x %d, orientation: %d, bpp: %d\n",
		// 	JPEG_getWidth(&jpeg),
		// 	JPEG_getHeight(&jpeg),
		// 	JPEG_getOrientation(&jpeg),
		// 	JPEG_getBpp(&jpeg)
		// );
		JPEG_decode(&jpeg, x, y, 0);
	}
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

    printf("\n\n\nPICO DM QD3503728 UDD JPEGDEC Perf test\n");

    ili9488_driver_init();
    backlight_driver_init();
    backlight_set_level(100);
    printf("backlight set to 100%%\n");

    printf("Running *JPEGDEC* decode test...\n");

//     jpegenc_drawjpg(0, 0, screen_480x320, sizeof(screen_480x320));
//     for(;;);

    uint sum_time = 0;
    for (;;) {
        start_time = time_us_32();

        /* Do JPEG decode here */
	jpegenc_drawjpg(0, 0, screen_480x320, sizeof(screen_480x320));

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
    printf("| *JPEGDEC* decode avg. time: %d us | FPS : %d |\n",
        sum_time / frame_count, 1000000 / (sum_time / frame_count));

    return 0;
}
