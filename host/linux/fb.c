// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2025 embeddedboys, Ltd.
 *
 * Author: Hua Zheng <hua.zheng@embeddedboys.com>
 */

#define DRV_NAME "udd-fb"
#define pr_fmt(fmt) DRV_NAME": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fb.h>

#include "udd.h"
#include "encoder.h"

struct dirty_area {
    u32 x1;
    u32 y1;
    u32 x2;
    u32 y2;
};

static ssize_t udd_fb_read(struct fb_info *info, char __user *buf,
			   size_t count, loff_t *ppos)
{
    pr_info("%s\n", __func__);
    return fb_sys_read(info, buf, count, ppos);
}

static ssize_t udd_fb_write(struct fb_info *info, const char __user *buf,
			    size_t count, loff_t *ppos)
{
    ssize_t ret = 0;
    pr_info("%s: count=%zd, ppos=%llu\n", __func__,  count, *ppos);
    ret = fb_sys_write(info, buf, count, ppos);
    schedule_delayed_work(&info->deferred_work, info->fbdefio->delay);
    return ret;
}

static void udd_fb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
    pr_info("%s\n", __func__);
    sys_fillrect(info, rect);
}

static void udd_fb_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
    pr_info("%s\n", __func__);
    sys_copyarea(info, area);
}

static void udd_fb_imageblit(struct fb_info *info, const struct fb_image *image)
{
    pr_info("%s\n", __func__);
    sys_imageblit(info, image);
}

/* from pxafb.c */
static unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
    chan &= 0xffff;
    chan >>= 16 - bf->length;
    return chan << bf->offset;
}

static int udd_fb_setcolreg(unsigned int regno, unsigned int red,
                                unsigned int green, unsigned int blue,
                                unsigned int transp, struct fb_info *info)
{
    unsigned int val;
    int ret = 1;

    pr_info("%s(regno=%u, red=0x%X, green=0x%X, blue=0x%X, trans=0x%X)\n",
           __func__, regno, red, green, blue, transp);

    if (regno >= 256)   /* no. of hw registers */
        return 1;

    switch (info->fix.visual) {
    case FB_VISUAL_TRUECOLOR:
        if (regno < 16) {
            val  = chan_to_field(red, &info->var.red);
            val |= chan_to_field(green, &info->var.green);
            val |= chan_to_field(blue, &info->var.blue);

            ((u32 *)(info->pseudo_palette))[regno] = val;
            ret = 0;
        }
        break;
    }

    return ret;
}

static int udd_fb_blank(int blank, struct fb_info *info)
{
    int ret = -EINVAL;

    switch (blank) {
    case FB_BLANK_POWERDOWN:
    case FB_BLANK_VSYNC_SUSPEND:
    case FB_BLANK_HSYNC_SUSPEND:
    case FB_BLANK_NORMAL:
        pr_info("%s, blank\n", __func__);
        break;
    case FB_BLANK_UNBLANK:
        pr_info("%s, unblank\n", __func__);
        break;
    }
    return ret;
}

static void udd_fb_deferred_io(struct fb_info *info, struct list_head *pagereflist)
{
    ssize_t jpeg_length = 0;
    // struct fb_deferred_io_pageref *pageref;
    // struct dirty_area area = {0};
    // uint y_cur, y_end;
    struct udd *udd;
    u8 *jpeg_data;

    udd = info->par;

// TODO: support partial update
#ifdef SUPPORT_PARTIAL_UPDATE
    area.y1 = info->var.yres - 1;
    list_for_each_entry(pageref, pagereflist, list) {
        y_cur = pageref->offset / info->fix.line_length;
        y_end = (pageref->offset + PAGE_SIZE) / info->fix.line_length;

        if (y_end > info->var.yres - 1)
            y_end = info->var.yres - 1;
        if (y_cur < area.y1)
            area.y1 = y_cur;
        if (y_end > area.y2)
            area.y2 = y_end;
    }

    if (y_end > 1)
        area.x2 = info->var.xres - 1;

    pr_info("%s, dirty area: (%d, %d, %d, %d)\n", __func__, area.x1, area.y1, area.x2, area.y2);
#endif


    jpeg_data = jpeg_encode_rgb565(info->screen_buffer,
                                info->fix.line_length * info->var.yres, &jpeg_length);

    if (jpeg_length > USB_TRANS_MAX_SIZE)
        goto skip_frame;

    udd_flush(udd->udev, jpeg_data, jpeg_length);

skip_frame:
    kfree(jpeg_data);
}

struct fb_info *udd_framebuffer_alloc(struct udd_display *display,
                                      struct device *dev)
{
    struct fb_deferred_io *fbdefio;
    struct fb_ops *fbops;
    struct fb_info *info;
    int width, height, bpp, rotate;
    u8 *vmem = NULL;
    int vmem_size;

    pr_info("%s\n", __func__);

    width  = display->xres;
    height = display->yres;
    bpp    = display->bpp;
    rotate = display->rotate;

    vmem_size = (width * height * bpp) / BITS_PER_BYTE;
    pr_info("vmem_size: %d\n", vmem_size);
    vmem = vzalloc(vmem_size);
    if (!vmem) {
        pr_err("failed to allocate vmem\n");
        return NULL;
    }

    fbops = kzalloc(sizeof(*fbops), GFP_KERNEL);
    if (!fbops) {
        pr_err("failed to allocate fbops\n");
        goto err_free_vmem;
    }

    fbdefio = kzalloc(sizeof(*fbdefio), GFP_KERNEL);
    if (!fbdefio) {
        pr_err("failed to allocate fbdefio\n");
        goto err_free_fbops;
    }

    info = framebuffer_alloc(sizeof(struct udd), dev);
    if (!info) {
        pr_err("failed to allocate info\n");
        goto err_free_fbdefio;
    }

    // info->dev = dev;
    info->screen_buffer = vmem;
    info->fbops = fbops;
    info->fbdefio = fbdefio;

    fbops->owner        = THIS_MODULE;
    fbops->fb_read      = udd_fb_read,
    fbops->fb_write     = udd_fb_write;
    fbops->fb_fillrect  = udd_fb_fillrect;
    fbops->fb_copyarea  = udd_fb_copyarea;
    fbops->fb_imageblit = udd_fb_imageblit;
    fbops->fb_setcolreg = udd_fb_setcolreg;
    fbops->fb_blank     = udd_fb_blank;

// TODO: Find out which version requires mmap to be implemented.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
    fbops->fb_mmap      = fb_deferred_io_mmap;
#endif

    snprintf(info->fix.id, sizeof(info->fix.id), "%s", DRV_NAME);
    info->fix.type        = FB_TYPE_PACKED_PIXELS;
    info->fix.visual      = FB_VISUAL_TRUECOLOR;
    info->fix.xpanstep    = 0;
    info->fix.ypanstep    = 0;
    info->fix.ywrapstep   = 0;
    info->fix.line_length = width * bpp / BITS_PER_BYTE;
    info->fix.accel       = FB_ACCEL_NONE;
    info->fix.smem_len    = vmem_size;

    info->var.rotate         = rotate;
    info->var.xres           = width;
    info->var.yres           = height;
    info->var.xres_virtual   = info->var.xres;
    info->var.yres_virtual   = info->var.yres;
    info->var.bits_per_pixel = bpp;
    info->var.nonstd         = 1;
    info->var.grayscale      = 0;

    info->var.red.offset    = 11;
    info->var.red.length    = 5;
    info->var.green.offset  = 5;
    info->var.green.length  = 6;
    info->var.blue.offset   = 0;
    info->var.blue.length   = 5;
    info->var.transp.offset = 0;
    info->var.transp.length = 0;

    info->flags = FBINFO_VIRTFB;

    fbdefio->delay = HZ / display->fps;
    fbdefio->sort_pagereflist = true;
    fbdefio->deferred_io = udd_fb_deferred_io;
    fb_deferred_io_init(info);

    return info;

err_free_fbdefio:
    kfree(fbdefio);
err_free_fbops:
    kfree(fbops);
err_free_vmem:
    vfree(vmem);
    return NULL;
}

void udd_framebuffer_release(struct fb_info *info)
{
    fb_deferred_io_cleanup(info);
    vfree(info->screen_buffer);
    framebuffer_release(info);
}

int udd_register_framebuffer(struct fb_info *info)
{
    int rc;
    rc = register_framebuffer(info);
    return rc;
}

int udd_unregister_framebuffer(struct fb_info *info)
{
    unregister_framebuffer(info);
    return 0;
}

// static int __init udd_fb_init(void)
// {
//     pr_info("%s\n", __func__);

//     dfb = kzalloc(sizeof(*dfb), GFP_KERNEL);
//     if (!dfb) {
//         pr_err("failed to allocate dfb\n");
//         return -ENOMEM;
//     };

//     dfb->class = class_create(THIS_MODULE, DRV_NAME "class");
//     if (IS_ERR(dfb->class)) {
//         pr_err("failed to create class\n");
//         goto err_free_dfb;
//     }

//     dfb->dev = device_create(dfb->class, NULL, MKDEV(0, 0), NULL, DRV_NAME "dev");
//     if (IS_ERR(dfb->dev)) {
//         pr_err("failed to create device\n");
//         goto err_free_class;
//     }

//     udd_fb_alloc(dfb);
//     return 0;

// err_free_class:
//     class_destroy(dfb->class);
// err_free_dfb:
//     kfree(dfb);
//     return -ENOMEM;
// }

// static void __exit udd_fb_exit(void)
// {
//     pr_info("%s\n", __func__);

//     if (dfb->dev)
//         device_destroy(dfb->class, MKDEV(0, 0));

//     if (dfb->class)
//         class_destroy(dfb->class);

//     if (dfb->info) {
//         fb_deferred_io_cleanup(dfb->info);
//         unregister_framebuffer(dfb->info);
//         vfree(dfb->info->screen_buffer);
//         framebuffer_release(dfb->info);
//     }

//     kfree(dfb);
// }
