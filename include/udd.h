// Copyright (c) 2025 embeddedboys developers
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

#ifndef __UDD_H
#define __UDD_H

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "tjpgd/tjpgd.h"
#include "jpegdec/JPEGDEC.h"

#define REQ_EP0_OUT 0X00
#define REQ_EP0_IN 0X01
#define REQ_EP1_OUT 0X02
#define REQ_EP2_IN 0X03
#define REQ_EP3_OUT 0X04
#define REQ_EP4_IN 0X05

struct tp_data {
	bool is_pressed;
	uint16_t x;
	uint16_t y;
};

struct tjpgd_data {
	const uint8_t* array_data;
	uint32_t array_index;
	uint32_t array_size;
	int16_t jpeg_x;
	int16_t jpeg_y;
};

struct jpegdec_data {
	JPEGIMAGE img;
	uint8_t options;
};

struct udd_data {

	/* UDD protocal data */
	uint8_t cmd;
	uint8_t tp_polling_period;

	/* Touchscreen data */
	struct tp_data tp;

	/* docoders data */
	struct tjpgd_data tjpgd;
	struct jpegdec_data jpegdec;

};

extern struct udd_data g_udd_data;

#endif // __UDD_H
