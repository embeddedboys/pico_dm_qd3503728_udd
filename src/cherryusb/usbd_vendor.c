#include "usbd_core.h"
#include "usbd_vendor.h"

static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
	if (index > 3) {
		return NULL;
	}
	return string_descriptors[index];
}

const struct usb_descriptor xxx_vendor_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
	.string_descriptor_callback = string_descriptor_callback
};

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t ep1_read_buffer[65536];

static int vendor_request_handler(uint8_t busid, struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
    // USB_LOG_WRN("%s, XXX Class request: "
    //              "bRequest 0x%02x\r\n",
    //              __func__,
    //              setup->bRequest);

    switch (setup->bRequest) {
        case REQ_EP1_OUT:
            // USB_LOG_WRN("REQ_EP1_OUT : 0x%02x, len : %d\r\n", setup->bRequest, *len);
            // usb_hexdump(*data, *len);
            uint16_t transfer_size;
            memcpy(&transfer_size, *data, sizeof(transfer_size));
            // printf("%s, ep1_out setup size : %d\n", __func__, transfer_size);
            usbd_ep_start_read(busid, EP1_OUT_ADDR, ep1_read_buffer, transfer_size);
            // usb_hexdump(ep1_read_buffer, transfer_size);
            return 0;
        default:
            USB_LOG_WRN("Unhandled XXX Class bRequest 0x%02x\r\n", setup->bRequest);
            return -1;
    }

    return 0;
}

static void vendor_notify_handler(uint8_t busid, uint8_t event, void *arg)
{
    switch (event) {
        case USBD_EVENT_RESET:
            USB_LOG_WRN("USBD_EVENT_RESET\r\n");
            break;

        default:
            break;
    }
}

struct usbd_interface *usbd_vendor_init_intf(uint8_t busid, struct usbd_interface *intf)
{
    intf->class_interface_handler = NULL;
    intf->class_endpoint_handler = NULL;
    intf->vendor_handler = vendor_request_handler;
    intf->notify_handler = vendor_notify_handler;

    return intf;
}
