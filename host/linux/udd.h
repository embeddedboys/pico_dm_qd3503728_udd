// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2025 embeddedboys, Ltd.
 *
 * Author: Zheng Hua <hua.zheng@embeddedboys.com>
 */

#ifndef __UDD_H
#define __UDD_H

#include <linux/kernel.h>

#include <drm/drm_drv.h>
#include <drm/drm_device.h>
#include <drm/drm_managed.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_fbdev_generic.h>
#include <drm/drm_probe_helper.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_damage_helper.h>
#include <drm/drm_gem_dma_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>

/* Display backends select, fbdev is default */
#define UDD_DISP_BACKEND_FBDEV 0
#define UDD_DISP_BACKEND_DRM   1

#ifndef UDD_DEF_DISP_BACKEND
    #define UDD_DEF_DISP_BACKEND UDD_DISP_BACKEND_FBDEV
#endif

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
    u64 dma_mask;
    struct device          *dev;

    /* USB specific data */
    struct usb_device      *udev;

    /* Framebuffer specific data */
    struct fb_info        *info;
    struct udd_display    *display;

    /* DRM specific data */
    u16 *tx_buf;
    u32 pixel_format;
    struct drm_device drm;
    struct drm_simple_display_pipe pipe;
    struct drm_connector connector;
    struct drm_display_mode mode;
};

struct fb_info *udd_framebuffer_alloc(struct udd_display *display,
                                      struct device *dev);
void udd_framebuffer_release(struct fb_info *info);
int udd_register_framebuffer(struct fb_info *info);
int udd_unregister_framebuffer(struct fb_info *info);

struct drm_device *udd_drm_alloc(struct device *dev);
void udd_drm_release(struct drm_device *drm);
int udd_drm_register(struct drm_device *drm);
void udd_drm_unregister(struct drm_device *drm);

ssize_t udd_flush(struct usb_device *udev, const u8 jpeg_data[], size_t data_size);

#endif
