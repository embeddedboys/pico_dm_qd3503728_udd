// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2025 embeddedboys, Ltd.
 *
 * Author: Zheng Hua <hua.zheng@embeddedboys.com>
 */

#define pr_fmt(fmt) "udd-drm: " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <video/mipi_display.h>

#include "udd.h"
#include "encoder.h"

#define DRV_NAME "udd-drm"

static inline struct udd *drm_to_udd(struct drm_device *drm)
{
    return container_of(drm, struct udd, drm);
}

static enum drm_mode_status udd_drm_pipe_mode_valid(struct drm_simple_display_pipe *pipe,
					      const struct drm_display_mode *mode)
{
    struct udd *udd = drm_to_udd(pipe->crtc.dev);
    int rc;
    rc = drm_crtc_helper_mode_valid_fixed(&pipe->crtc, mode, &udd->mode);
    pr_info("%s, rc: %d\n", __func__, rc);
    return rc;
}

static void udd_drm_pipe_enable(struct drm_simple_display_pipe *pipe,
				  struct drm_crtc_state *crtc_state,
				  struct drm_plane_state *plane_state)
{
    pr_info("%s\n", __func__);
}

static void udd_drm_pipe_disable(struct drm_simple_display_pipe *pipe)
{
    pr_info("%s\n", __func__);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
static int udd_buf_copy(void *dst, struct iosys_map *src, struct drm_framebuffer *fb,
                        struct drm_rect *clip, bool swap,
                        struct drm_format_conv_state *fmtcnv_state)
#else
static int udd_buf_copy(void *dst, struct iosys_map *src, struct drm_framebuffer *fb,
                        struct drm_rect *clip, bool swap)
#endif
{
    struct udd *udd = drm_to_udd(fb->dev);
    struct drm_gem_object *gem = drm_gem_fb_get_obj(fb, 0);
    struct iosys_map dst_map = IOSYS_MAP_INIT_VADDR(dst);
    int ret;

    ret = drm_gem_fb_begin_cpu_access(fb, DMA_FROM_DEVICE);
    if (ret)
        return ret;

    switch (fb->format->format) {
    case DRM_FORMAT_RGB565:
        if (swap)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
            drm_fb_swab(&dst_map, NULL, src, fb, clip, !gem->import_attach,
                        fmtcnv_state);
#else
            drm_fb_swab(&dst_map, NULL, src, fb, clip, !gem->import_attach);
#endif

        else
            drm_fb_memcpy(&dst_map, NULL, src, fb, clip);
        break;
    case DRM_FORMAT_RGB888:
        drm_fb_memcpy(&dst_map, NULL, src, fb, clip);
        break;
    case DRM_FORMAT_XRGB8888:
        switch (udd->pixel_format) {
        case DRM_FORMAT_RGB565:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
            drm_fb_xrgb8888_to_rgb565(&dst_map, NULL, src, fb, clip, fmtcnv_state, swap);
#else
            drm_fb_xrgb8888_to_rgb565(&dst_map, NULL, src, fb, clip, swap);
#endif
            break;
        case DRM_FORMAT_RGB888:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
            drm_fb_xrgb8888_to_rgb888(&dst_map, NULL, src, fb, clip, fmtcnv_state);
#else
            drm_fb_xrgb8888_to_rgb888(&dst_map, NULL, src, fb, clip);
#endif
            break;
        }
        break;
    default:
        drm_err_once(fb->dev, "Format is not supported: %p4cc\n",
            &fb->format->format);
        ret = -EINVAL;
    }

    drm_gem_fb_end_cpu_access(fb, DMA_FROM_DEVICE);

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
static void udd_fb_dirty(struct iosys_map *src, struct drm_framebuffer *fb,
                        struct drm_rect *rect, struct drm_format_conv_state *fmtcnv_state)
#else
static void udd_fb_dirty(struct iosys_map *src, struct drm_framebuffer *fb,
                        struct drm_rect *rect)
#endif
{
    struct udd *udd = drm_to_udd(fb->dev);
    unsigned int height = rect->y2 - rect->y1;
    unsigned int width = rect->x2 - rect->x1;
    // const struct drm_format_info *dst_format;
    ssize_t jpeg_length = 0;
    u8 *jpeg_data;
    bool swap = false;
    int ret = 0;
    // size_t len;
    bool full;
    void *tr;

    full = width == fb->width && height == fb->height;

    tr = udd->tx_buf;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
    ret = udd_buf_copy(tr, src, fb, rect, swap, fmtcnv_state);
#else
    ret = udd_buf_copy(tr, src, fb, rect, swap);
#endif
    if (ret) {
        pr_info("%s, error on buf copy!\n", __func__);
    }
    // tr = src->vaddr;

    jpeg_data = jpeg_encode_rgb565(tr,
                                480 * 320, &jpeg_length);

    pr_info("%s, len : %ld\n", __func__, jpeg_length);
    if (jpeg_length > USB_TRANS_MAX_SIZE)
        // goto skip_frame;
        jpeg_length = USB_TRANS_MAX_SIZE - 1;

    udd_flush(udd->udev, jpeg_data, jpeg_length);
// skip_frame:
    kfree(jpeg_data);
}

static void udd_drm_pipe_update(struct drm_simple_display_pipe *pipe,
                                struct drm_plane_state *old_state)
{
    struct drm_plane_state *state = pipe->plane.state;
    struct drm_shadow_plane_state *shadow_plane_state = to_drm_shadow_plane_state(state);
    struct drm_framebuffer *fb = state->fb;
    struct drm_rect rect, full_rect;
    int idx;

    if (!pipe->crtc.state->active)
        return;

    if (WARN_ON(!fb))
        return;

    if (!drm_dev_enter(fb->dev, &idx))
        return;

    // NOTE: use full refresh temporarily
    full_rect.x1 = 0;
    full_rect.y1 = 0;
    full_rect.x2 = 480;
    full_rect.y2 = 320;

    pr_info("%s\n", __func__);
    if (drm_atomic_helper_damage_merged(old_state, state, &rect)) {
        pr_info("x1: %u, y1: %u, x2: %u, y2: %u\n", rect.x1, rect.y1, rect.x2, rect.y2);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
        udd_fb_dirty(&shadow_plane_state->data[0], fb, &full_rect,
                    &shadow_plane_state->fmtcnv_state);
#else
        udd_fb_dirty(&shadow_plane_state->data[0], fb, &full_rect);
#endif
    }

    drm_dev_exit(idx);
}

static int udd_drm_pipe_begin_fb_access(struct drm_simple_display_pipe *pipe,
				  struct drm_plane_state *plane_state)
{
    return drm_gem_begin_shadow_fb_access(&pipe->plane, plane_state);
}

static void udd_drm_pipe_end_fb_access(struct drm_simple_display_pipe *pipe,
				 struct drm_plane_state *plane_state)
{
	drm_gem_end_shadow_fb_access(&pipe->plane, plane_state);
}

static void udd_drm_pipe_reset_plane(struct drm_simple_display_pipe *pipe)
{
	drm_gem_reset_shadow_plane(&pipe->plane);
}

static struct drm_plane_state *udd_drm_pipe_duplicate_plane_state(struct drm_simple_display_pipe *pipe)
{
	return drm_gem_duplicate_shadow_plane_state(&pipe->plane);
}

static void udd_drm_pipe_destroy_plane_state(struct drm_simple_display_pipe *pipe,
				       struct drm_plane_state *plane_state)
{
	drm_gem_destroy_shadow_plane_state(&pipe->plane, plane_state);
}

static const struct drm_simple_display_pipe_funcs udd_display_pipe_funcs = {
    .mode_valid = udd_drm_pipe_mode_valid,
    .enable = udd_drm_pipe_enable,
    .disable = udd_drm_pipe_disable,
    .update = udd_drm_pipe_update,
    .begin_fb_access = udd_drm_pipe_begin_fb_access,
    .end_fb_access = udd_drm_pipe_end_fb_access,
    .reset_plane = udd_drm_pipe_reset_plane,
    .duplicate_plane_state = udd_drm_pipe_duplicate_plane_state,
    .destroy_plane_state = udd_drm_pipe_destroy_plane_state,
};

static int udd_connector_get_modes(struct drm_connector *connector)
{
	struct udd *udd = drm_to_udd(connector->dev);

	return drm_connector_helper_get_modes_fixed(connector, &udd->mode);
}

static const struct drm_connector_helper_funcs udd_connector_hfuncs = {
    .get_modes = udd_connector_get_modes,
};

static const struct drm_connector_funcs udd_connector_funcs = {
    .reset = drm_atomic_helper_connector_reset,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.destroy = drm_connector_cleanup,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

static const struct drm_mode_config_funcs udd_drm_mode_config_funcs = {
    .fb_create = drm_gem_fb_create_with_dirty,
    .atomic_check = drm_atomic_helper_check,
    .atomic_commit = drm_atomic_helper_commit,
};

static const uint32_t udd_drm_formats[] = {
    DRM_FORMAT_RGB565,
    DRM_FORMAT_XRGB8888,
};

static const struct drm_display_mode udd_disp_mode = {
    DRM_MODE_INIT(60, 480, 320, 85, 55),
};

DEFINE_DRM_GEM_DMA_FOPS(udd_drm_fops);

static const struct drm_driver udd_drm_driver = {
    .driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
    .fops = &udd_drm_fops,
    DRM_GEM_DMA_DRIVER_OPS_VMAP,
    .name = "udd-drm",
    .desc = "UDD DRM driver",
    .date = "20250119",
    .major = 1,
    .minor = 0,
};

static int udd_drm_dev_init_with_formats(struct udd *udd,
                const struct drm_simple_display_pipe_funcs *funcs,
                const uint32_t *formats, unsigned int formats_count,
                const struct drm_display_mode *mode, size_t tx_buf_size)
{
    static const uint64_t modifiers[] = {
		DRM_FORMAT_MOD_LINEAR,
		DRM_FORMAT_MOD_INVALID
	};
    struct drm_device *drm = &udd->drm;
    int rc;

    pr_info("%s\n", __func__);

    rc = drm_mode_config_init(drm);
    if (rc) {
        pr_err("failed to init mode config\n");
        return rc;
    }

    udd->tx_buf = devm_kmalloc(drm->dev, tx_buf_size, GFP_KERNEL);
    if (!udd->tx_buf)
        return -ENOMEM;

    drm_mode_copy(&udd->mode, mode);
    pr_info("mode: %ux%u\n", udd->mode.hdisplay, udd->mode.vdisplay);

    drm_connector_helper_add(&udd->connector, &udd_connector_hfuncs);
    rc = drm_connector_init(drm, &udd->connector, &udd_connector_funcs,
                            DRM_MODE_CONNECTOR_USB);
    if (rc) {
        pr_err("failed to init connector\n");
        return rc;
    }

    rc = drm_simple_display_pipe_init(drm, &udd->pipe, funcs, formats, formats_count, modifiers, &udd->connector);
    if (rc) {
        pr_err("failed to init pipe\n");
        return rc;
    }

    drm_plane_enable_fb_damage_clips(&udd->pipe.plane);

    drm->mode_config.funcs = &udd_drm_mode_config_funcs;
    drm->mode_config.min_width = udd->mode.hdisplay;
    drm->mode_config.max_width = udd->mode.hdisplay;
    drm->mode_config.min_height = udd->mode.vdisplay;
    drm->mode_config.max_height = udd->mode.vdisplay;
    udd->pixel_format = formats[0];

    DRM_DEBUG_KMS("mode: %ux%u", udd->mode.hdisplay, udd->mode.vdisplay);

    return 0;
}

static int udd_drm_dev_init(struct udd *udd,
                const struct drm_simple_display_pipe_funcs *funcs,
                const struct drm_display_mode *mode)
{
    ssize_t bufsize = mode->vdisplay * mode->hdisplay * sizeof(u16);

    udd->drm.mode_config.preferred_depth = 16;

    pr_info("%s\n", __func__);

    return udd_drm_dev_init_with_formats(udd, funcs, udd_drm_formats,
                        ARRAY_SIZE(udd_drm_formats), mode, bufsize);
}

struct drm_device *udd_drm_alloc(struct device *dev)
{
    struct udd *udd;
    struct drm_device *drm;
    int rc;

    pr_info("%s\n", __func__);
    udd = devm_drm_dev_alloc(dev, &udd_drm_driver,
                    struct udd, drm);
    if (IS_ERR(udd)) {
        pr_err("failed to allocate drm\n");
        return ERR_PTR(-ENOMEM);
    }
    drm = &udd->drm;

    udd->dma_mask = DMA_BIT_MASK(32);
    dev->dma_mask = &udd->dma_mask;
    dev->coherent_dma_mask = udd->dma_mask;

    rc = udd_drm_dev_init(udd, &udd_display_pipe_funcs, &udd_disp_mode);
    if (rc) {
        pr_err("failed to init drm dev\n");
        return ERR_PTR(-ENOMEM);
    }

    return drm;
}

void udd_drm_release(struct drm_device *drm)
{
    pr_info("%s\n", __func__);
}

int udd_drm_register(struct drm_device *drm)
{
    int rc;

    pr_info("%s\n", __func__);

    drm_mode_config_reset(drm);

    rc = drm_dev_register(drm, 0);
    if (rc) {
        pr_err("failed to register drm dev\n");
        return -1;
    };

    drm_fbdev_generic_setup(drm, 0);

    return 0;
}

void udd_drm_unregister(struct drm_device *drm)
{
    pr_info("%s\n", __func__);
    drm_dev_unplug(drm);
    drm_atomic_helper_shutdown(drm);
}
