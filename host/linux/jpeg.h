#ifndef __JPEG__
#define __JPEG__

#include <linux/kernel.h>

uint8_t *encode_bmp(uint8_t *bmp, size_t len, size_t *out_size);

#endif