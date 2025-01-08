#ifndef __UDD_H
#define __UDD_H

#include <linux/kernel.h>

// TODO: Currently only support less than 40000 bytes transfer
#define USB_TRANS_MAX_SIZE  40000

struct udd_display {
    u32     xres;
    u32     yres;
    u32     bpp;
    u32     fps;
    u32     rotate;
};

struct udd {
    struct device          *dev;

    /* USB specific data */
    struct usb_device      *udev;

    /* Framebuffer specific data */
    struct fb_info        *info;
    struct udd_display    *display;
};

struct fb_info *udd_framebuffer_alloc(struct udd_display *display,
                                      struct device *dev);
void udd_framebuffer_release(struct fb_info *info);
int udd_register_framebuffer(struct fb_info *info);
int udd_unregister_framebuffer(struct fb_info *info);

ssize_t udd_flush(struct usb_device *udev, const u8 jpeg_data[], size_t data_size);

#endif
