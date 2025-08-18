#ifndef __USB_H
#define __USB_H

#include <hardware/structs/usb.h>

#include <stdbool.h>

#include "usbd_core.h"
#include "usbd_vendor.h"

void usb_device_init(void);
bool usb_is_configured(void);

#endif /* __USB_H */