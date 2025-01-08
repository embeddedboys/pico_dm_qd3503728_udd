#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/input.h>

// A jpeg image of a panda, binary data
#include "panda.h"

#include "encoder.h"
#include "rgb565.h"

#define DRV_NAME "udd"
#define UDD_DEFAULT_TIMEOUT 1000

#define EP0_IN_ADDR  (USB_DIR_IN  | 0)
#define EP0_OUT_ADDR (USB_DIR_OUT | 0)
#define EP1_OUT_ADDR (USB_DIR_OUT | 1)
#define EP2_IN_ADDR  (USB_DIR_IN  | 2)

#define TYPE_VENDOR 0x40

#define REQ_EP0_OUT  0X00
#define REQ_EP0_IN   0X01
#define REQ_EP1_OUT  0X02
#define REQ_EP2_IN   0X03

static int udd_flush(struct usb_device *udev, const u8 jpeg_data[], size_t data_size)
{
    u8 control_buffer[4];
    int rc, actual_length;

    /* data_size must be even for RP2350 */
    if (data_size % 2)
        data_size += 1;

    control_buffer[0] = 0x51;
    control_buffer[1] = data_size & 0xff;
    control_buffer[2] = data_size >> 8;
    control_buffer[3] = 0x00;

    // request setup
    rc = usb_control_msg(
        udev,
        usb_sndctrlpipe(udev, EP0_OUT_ADDR),
        REQ_EP1_OUT,
        TYPE_VENDOR | USB_DIR_OUT,
        0, 0,
        control_buffer,
        sizeof(control_buffer),
        UDD_DEFAULT_TIMEOUT
    );

    rc = usb_bulk_msg(
        udev,
        usb_sndbulkpipe(udev, EP1_OUT_ADDR),
        (void *)jpeg_data,
        data_size,
        &actual_length,
        UDD_DEFAULT_TIMEOUT
    );

    return 0;
}

static int udd_probe(struct usb_interface *intf,
                    const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(intf);
    struct usb_endpoint_descriptor *endpoint_desc;
    struct usb_host_interface *interface;
    size_t actual_length;
    u8 *usb_buffer, *jpeg_data;

    printk("\n\n%s\n", __func__);

    interface = intf->cur_altsetting;
    endpoint_desc = &interface->endpoint[0].desc;
    // printk("num of eps : %d\n", interface->desc.bNumEndpoints);

    // printk("bLength : 0x%02x", endpoint_desc->bLength);
    // printk("bDescriptorType : 0x%02x", endpoint_desc->bDescriptorType);
    // printk("bEndpointAddress : 0x%02x\n", endpoint_desc->bEndpointAddress);
    // printk("bmAttributes : 0x%02x", endpoint_desc->bmAttributes);
    // printk("wMaxPacketSize : 0x%04x", endpoint_desc->wMaxPacketSize);
    // printk("bInterval : 0x%02x\n", endpoint_desc->bInterval);

    // Load jpeg data into memory
    usb_buffer = (u8 *)kmalloc(32 * 1024, GFP_KERNEL);
    if (!usb_buffer)
        return -ENOMEM;

    /* TODO: replace these with a booting up image */
    memcpy(usb_buffer, &panda, sizeof(panda));
    udd_flush(udev, usb_buffer, sizeof(panda));

    jpeg_data = jpeg_encode_bmp(rgb565, ARRAY_SIZE(rgb565), &actual_length);
    memcpy(usb_buffer, jpeg_data, actual_length);
    udd_flush(udev, usb_buffer, actual_length);

    kfree(usb_buffer);
    kfree(jpeg_data);

    return 0;
}
static void udd_disconnect(struct usb_interface *intf)
{
    printk("%s\n", __func__);
}

static struct usb_device_id udd_ids[] = {
    { USB_DEVICE(0x2E8A, 0x0001) },
    { /* KEEP THIS */ }
};
MODULE_DEVICE_TABLE(usb, udd_ids);

static struct usb_driver udd_drv = {
    .name       = DRV_NAME,
    .probe      = udd_probe,
    .disconnect = udd_disconnect,
    .id_table   = udd_ids,
};
module_usb_driver(udd_drv);

MODULE_AUTHOR("Hua Zheng <writeforever@foxmail.com>");
MODULE_DESCRIPTION("USB display device driver");
MODULE_LICENSE("GPL");
