#ifndef _USBD_XXX_H_
#define _USBD_XXX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_core.h"

#define VENDOR_ID  0x2E8A
#define PRODUCT_ID 0x0001

#define USBD_MAX_POWER 500

#define EP0_IN_ADDR  (USB_EP_DIR_IN  | 0)
#define EP0_OUT_ADDR (USB_EP_DIR_OUT | 0)
#define EP1_OUT_ADDR (USB_EP_DIR_OUT | 1)
#define EP2_IN_ADDR  (USB_EP_DIR_IN  | 2)
#define EP3_OUT_ADDR (USB_EP_DIR_OUT | 3)
#define EP4_IN_ADDR  (USB_EP_DIR_IN  | 4)

#define REQ_EP0_OUT     0X00
#define REQ_EP0_IN      0X01
#define REQ_EP1_OUT     0X02
#define REQ_EP2_IN      0X03
#define REQ_EP3_OUT     0X04
#define REQ_EP4_IN      0X05

// static const struct usb_device_descriptor device_descriptor = {
//     .bLength            = sizeof(struct usb_device_descriptor),
//     .bDescriptorType    = USB_DT_DEVICE,
//     .bcdUSB             = 0x0110,  // USB 1.1 device
//     .bDeviceClass       = 0,       // Specified in interface descriptor
//     .bDeviceSubClass    = 0,       // No subclass
//     .bDeviceProtocol    = 0,       // No protocol
//     .bMaxPacketSize0    = 64,      // Max packet size for ep0
//     .idVendor           = VENDOR_ID,   // Your vendor id
//     .idProduct          = PRODUCT_ID,  // Your product ID
//     .bcdDevice          = 0,       // No device revision number
//     .iManufacturer      = 1,       // Manufacturer string index
//     .iProduct           = 2,       // Product string index
//     .iSerialNumber      = 0,       // No serial number
//     .bNumConfigurations = 1        // One configuration
// };

// static struct usb_interface_descriptor interface_descriptor = {
//     .bLength            = sizeof(struct usb_interface_descriptor),
//     .bDescriptorType    = USB_DT_INTERFACE,
//     .bInterfaceNumber   = 0,
//     .bAlternateSetting  = 0,
//     .bInterfaceClass    = 0xff,  // Vendor specific endpoint
//     .bInterfaceSubClass = 0,
//     .bInterfaceProtocol = 0,
//     .iInterface         = 0
// };

// static struct usb_configuration_descriptor config_descriptor = {
//     .bLength             = sizeof(struct usb_configuration_descriptor),
//     .bDescriptorType     = USB_DT_CONFIG,
//     .bNumInterfaces      = 1,
//     .bConfigurationValue = 1,       // Configuration 1
//     .iConfiguration      = 0,       // No string
//     // .bmAttributes        = 0xc0,    // attributes: self powered, no remote wakeup
//     // .bMaxPower           = 0x32     // 100ma
//     .bmAttributes        = 0x80,    // attributes: bus powered
//     .bMaxPower           = 0xFA     // 500ma
// };

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0, 0, 0, VENDOR_ID, PRODUCT_ID, 0, 1),
};

static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(9 + 9 + 7, 1, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    USB_INTERFACE_DESCRIPTOR_INIT(0, 0, 1, 0xFF, 0, 0, 0),
    USB_ENDPOINT_DESCRIPTOR_INIT(EP1_OUT_ADDR, USB_ENDPOINT_TYPE_BULK, USB_BULK_EP_MPS_FS, 0)
};

static const uint8_t device_quality_descriptor[] = {
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x00,
    0x00,
};

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "embeddedboys",                  /* Manufacturer */
    "USB Display",         		/* Product */
    "20250818",                 /* Serial Number */
};

extern const struct usb_descriptor xxx_vendor_descriptor;

struct usbd_interface *usbd_vendor_init_intf(uint8_t busid, struct usbd_interface *intf);

#ifdef __cplusplus
}
#endif

#endif /* _USBD_XXX_H_ */
