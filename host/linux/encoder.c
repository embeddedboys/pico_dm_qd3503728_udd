#include <linux/slab.h>

#include "jpegenc.h"
uint8_t *jpeg_encode_bmp(uint8_t *bmp, size_t len, size_t *out_size)
{
    int rc, y, w, h, bits, offset;
    int pitch, bytewidth, delta;
    uint8_t *buffer, *bmp_tmp;
    size_t buffer_size;
    uint8_t *src, *dst;
    JPEGE_IMAGE jpeg;
    JPEGENCODE jpe;

    /* Sanity check */
    if (bmp[0] != 'B' || bmp[1] != 'M' || bmp[14] < 0x28) {
        printk("Not a BMP file\n");
        return NULL;
    }

    w = *(int32_t *)&bmp[18];
    h = *(int32_t *)&bmp[22];

    bits = *(int16_t *)&bmp[26] * *(int16_t *)&bmp[28];
    if (bits != 16) {
        printk("Not a 16-bit BMP file\n");
        return NULL;
    }

    offset = *(int32_t *)&bmp[10];
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc;
    printk("%s, w : %d, h : %d, pitch : %d\n", __func__, w, h, pitch);

    buffer_size = len;
    buffer = (uint8_t *)kmalloc(buffer_size, GFP_KERNEL);
    bmp_tmp = (uint8_t *)kmalloc(buffer_size, GFP_KERNEL);

    dst = bmp_tmp;
    src = &bmp[offset];
    delta = pitch;
    if (h > 0) {
        delta = -pitch;
        src = &bmp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }

    for (y = 0; y < h; y++) {
        memcpy(dst, src, bytewidth);
        dst += bytewidth;
        src += delta;
    }

    memset(&jpeg, 0, sizeof(JPEGE_IMAGE));
    jpeg.pOutput = buffer;
    jpeg.iBufferSize = buffer_size;
    jpeg.pHighWater = &jpeg.pOutput[jpeg.iBufferSize - 512];

    rc = JPEGEncodeBegin(&jpeg, &jpe, w, h, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_HIGH);
    if (rc == JPEGE_SUCCESS)
        JPEGAddFrame(&jpeg, &jpe, bmp_tmp, pitch);

    JPEGEncodeEnd(&jpeg);
    printk("%s, jpeg size : %d\n", __func__, jpeg.iDataSize);
    *out_size = jpeg.iDataSize;

    kfree(bmp_tmp);

    return buffer;
}
