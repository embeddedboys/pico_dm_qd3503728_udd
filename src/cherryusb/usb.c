#include "usb.h"

#include "decoder.h"

#include <pico/mutex.h>

mutex_t decoder_mutex;

void usbd_event_handler(uint8_t busid, uint8_t event)
{
	switch (event) {
		// case USBD_EVENT_RESET:
		// 	break;
		// case USBD_EVENT_CONNECTED:
		// 	break;
		// case USBD_EVENT_DISCONNECTED:
		// 	break;
		// case USBD_EVENT_RESUME:
		// 	break;
		// case USBD_EVENT_SUSPEND:
		// 	break;
		// case USBD_EVENT_CONFIGURED:
		// 	break;
		// case USBD_EVENT_SET_REMOTE_WAKEUP:
		// 	break;
		// case USBD_EVENT_CLR_REMOTE_WAKEUP:
		// 	break;
		default:
			USB_LOG_WRN("Unhandled event : %d, bus id : %d\n", event, busid);
			break;
	}
}

extern uint8_t ep1_read_buffer[65536];

void usbd_vendor_ep1_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
	// USB_LOG_RAW("%s, actual out len:%d\r\n", __func__, nbytes);
	decoder_drawimg(0, 0, ep1_read_buffer, nbytes);
}

struct usbd_endpoint vendor_out_ep1 = {
	.ep_addr = EP1_OUT_ADDR,
	.ep_cb = usbd_vendor_ep1_bulk_out,
};

struct usbd_interface intf0;

void usb_device_init()
{
	USB_LOG_WRN("%s\n", __func__);

	mutex_init(&decoder_mutex);

	usbd_desc_register(0, &xxx_vendor_descriptor);

	usbd_add_interface(0, usbd_vendor_init_intf(0, &intf0));

	usbd_add_endpoint(0, &vendor_out_ep1);

	usbd_initialize(0, USBCTRL_REGS_BASE, usbd_event_handler);

	// extern void cdc_acm_init(uint8_t busid, uintptr_t reg_base);
    	// cdc_acm_init(0, 0); // regbase is not used
}

bool usb_is_configured(void)
{
	return true;
}
