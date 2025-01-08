#ifndef __ENCODER_H
#define __ENCODER_H

#include <linux/kernel.h>

uint8_t *jpeg_encode_bmp(uint8_t *bmp, size_t len, size_t *out_size);
uint8_t *jpeg_encode_rgb565(uint8_t *rgb565, size_t len, size_t *out_size);

#endif