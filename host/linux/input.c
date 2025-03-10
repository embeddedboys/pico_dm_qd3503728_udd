// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 * Copyright (C) 2025 embeddedboys, Ltd.
 *
 * Author: Zheng Hua <hua.zheng@embeddedboys.com>
 */

#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/input/touchscreen.h>

#include "udd.h"

static void udd_tp_work(struct work_struct *work)
{
	struct udd *udd = container_of(work, struct udd, work);
	u8 control_buffer[4] = {0};
	int ret;

	usb_control_msg(
		udd->udev,
		usb_sndctrlpipe(udd->udev, EP0_OUT_ADDR),
		REQ_EP4_IN,
		TYPE_VENDOR | USB_DIR_OUT,
		0, 0,
		control_buffer,
		sizeof(control_buffer),
		UDD_DEFAULT_TIMEOUT
	);

	ret = usb_submit_urb(udd->input_urb, GFP_KERNEL);
	if (ret)
		printk("can't submit urb, %d\n", ret);
}

static void udd_tp_urb_callback(struct urb *urb)
{
	struct udd *udd = urb->context;
	struct input_dev *indev = udd->indev;
	bool pressed;
	u16 x, y;

	if (urb->status) {
		printk("%s, error status\n", __func__);
		return;
	}

	pressed = udd->ep_int_buf[0];
	x = udd->ep_int_buf[1] << 8 | udd->ep_int_buf[2];
	y = udd->ep_int_buf[3] << 8 | udd->ep_int_buf[4];

	input_report_key(indev, BTN_TOUCH, pressed);
	input_report_abs(indev, ABS_X, x);
	input_report_abs(indev, ABS_Y, y);
	// touchscreen_report_pos(indev, &udd->props, x, y, 0);

	input_sync(indev);

	schedule_work(&udd->work);
}

static int udd_tp_indev_open(struct input_dev *dev)
{
	/* Nothing needs to be done so far. */
	return 0;
}

static void udd_tp_indev_close(struct input_dev *dev)
{
	/* Nothing needs to be done so far. */
}

int udd_input_setup(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *endpoint_desc;
	struct usb_host_interface *interface;
	struct input_dev *input_dev;
	struct udd *udd;
	int pipe, maxp;
	int i, rc;

	udd = dev_get_drvdata(&intf->dev);

	interface = intf->cur_altsetting;
	// printk("num of eps : %d\n", interface->desc.bNumEndpoints);

	for (i = 0; i < interface->desc.bNumEndpoints; i++) {
		if (usb_endpoint_is_int_in(&interface->endpoint[i].desc))
			endpoint_desc = &interface->endpoint[i].desc;
	}

	if (!endpoint_desc) {
		printk("%s, device doesn't have a int endpoint for polling.\n", __func__);
		return -ENODEV;
	}

	udd->input_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!udd->input_urb)
	    return -ENOMEM;

	pipe = usb_rcvintpipe(udev, endpoint_desc->bEndpointAddress);
	maxp = usb_maxpacket(udd->udev, pipe);

	udd->ep_int_buf = kmalloc(maxp, GFP_KERNEL);
	if (!udd->ep_int_buf)
		goto free_urb;

	usb_fill_int_urb(
		udd->input_urb,
		udev,
		pipe,
		udd->ep_int_buf,
		maxp,
		udd_tp_urb_callback,
		udd,
		endpoint_desc->bInterval
	);

	INIT_WORK(&udd->work, udd_tp_work);

	input_dev = devm_input_allocate_device(&intf->dev);
	if (!input_dev) {
		printk("Failed to allocate input dev.\n");
		return -ENOMEM;
	}

	input_dev->dev.parent = &intf->dev;
	input_dev->name = "UDD touch panel";
	input_dev->id.bustype = BUS_USB;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	/* TODO: query from device via udd protocal */
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 320, 0, 0);

	/* TODO: multitouch support  */
	input_mt_init_slots(input_dev, 1,
		INPUT_MT_DIRECT | INPUT_MT_TRACK |
		    INPUT_MT_DROP_UNUSED);

	input_set_drvdata(input_dev, udd);

	input_dev->open = udd_tp_indev_open;
	input_dev->close = udd_tp_indev_close;

	udd->indev = input_dev;

	rc = input_register_device(input_dev);
	if (rc) {
		printk("Failed to register input dev.\n");
		goto free_buf;
	}

	schedule_work(&udd->work);
	return 0;

free_buf:
	kfree(udd->ep_int_buf);
free_urb:
	usb_free_urb(udd->input_urb);
	return -1;
}

int udd_input_cleanup(struct usb_interface *intf)
{
	struct udd *udd = dev_get_drvdata(&intf->dev);

	usb_kill_urb(udd->input_urb);
	input_unregister_device(udd->indev);
	usb_free_urb(udd->input_urb);

	kfree(udd->ep_int_buf);

	return 0;
}
