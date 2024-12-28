#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/input.h>

#include "panda.h"

#define DRV_NAME "udd"

static int udd_probe(struct usb_interface *intf,
                    const struct usb_device_id *id)
{
    struct usb_device *udev = interface_to_usbdev(intf);
    struct usb_endpoint_descriptor *endpoint_desc;
    struct usb_host_interface *interface;
    void *transfer_buffer;
    dma_addr_t phy_addr;
    unsigned int pipe;
    struct urb *urb;
    int rc;

    printk("%s\n", __func__);

    interface = intf->cur_altsetting;

    printk("num of eps : %d\n", interface->desc.bNumEndpoints);

    endpoint_desc = &interface->endpoint[0].desc;

    printk("bLength : 0x%02x", endpoint_desc->bLength);
    printk("bDescriptorType : 0x%02x", endpoint_desc->bDescriptorType);
    printk("bEndpointAddress : 0x%02x\n", endpoint_desc->bEndpointAddress);
    printk("bmAttributes : 0x%02x", endpoint_desc->bmAttributes);
    printk("wMaxPacketSize : 0x%04x", endpoint_desc->wMaxPacketSize);
    printk("bInterval : 0x%02x\n", endpoint_desc->bInterval);

    // pipe = usb_rcvintpipe(udev, endpoint_desc->bEndpointAddress);

    return 0;

    // transfer_buffer = usb_alloc_coherent(udev, 4, GFP_KERNEL, &phy_addr);
    // urb = usb_alloc_urb(0, GFP_KERNEL);

    // usb_fill_control_urb(urb, udev, )
    // return 0;
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
