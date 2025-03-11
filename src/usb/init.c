/**
 * Copyright (c) 2024 Daniel Gorbea
 *
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd. author of https://github.com/raspberrypi/pico-examples/tree/master/usb
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "udd.h"
#include "usb.h"
#include "ft6236.h"
#include "debug.h"
#include "decoder.h"

#define TAG "usb: "

uint start_time, end_time;

void control_transfer_handler(uint8_t *buf, volatile struct usb_setup_packet *pkt, uint8_t stage) {
    // pr_debug("\nControl transfer. Stage %u bmRequestType 0x%x bRequest 0x%x wValue 0x%x wIndex 0x%x wLength %u", stage,
    //        pkt->bmRequestType, pkt->bRequest, pkt->wValue, pkt->wIndex, pkt->wLength);

    // Setup stage.
    switch (stage) {
    case STAGE_SETUP:
        if (pkt->bmRequestType == USB_DIR_OUT) {
            if (pkt->bRequest == USB_REQUEST_SET_ADDRESS)
                pr_debug("Set address %u\n", usb_get_address());
            else if (pkt->bRequest == USB_REQUEST_SET_CONFIGURATION)
                pr_debug("Device Enumerated\n");
        }
        else if (pkt->bmRequestType & USB_REQ_TYPE_TYPE_VENDOR) {
            if (pkt->bRequest == REQ_EP0_IN) {
                for (uint i = 0; i < pkt->wLength; i++) buf[i] = i;
            }
        }
        break;
    case STAGE_DATA:
        if (pkt->bmRequestType == USB_DIR_IN) {
            if (pkt->bRequest == USB_REQUEST_GET_DESCRIPTOR) {
                uint16_t descriptor_type = pkt->wValue >> 8;
                switch (descriptor_type) {
                    case USB_DT_DEVICE:
                        pr_debug("Sent device descriptor\n");
                        break;
                    case USB_DT_CONFIG:
                        pr_debug("Sent config descriptor\n");
                        break;
                    case USB_DT_STRING:
                        pr_debug("Sent string descriptor\n");
                        break;
                }
            }
        } else if (pkt->bmRequestType & USB_REQ_TYPE_TYPE_VENDOR) {
            if (pkt->bRequest == REQ_EP0_OUT) {

                pr_debug("Received request REQ_EP0_OUT, length %u\n", pkt->wLength);
                for (uint i = 0; i < pkt->wLength; i++) pr_debug("\n%u ", buf[i]);
                pr_debug("\n");

            } else if (pkt->bRequest == REQ_EP0_IN) {

                pr_debug("Sent request REQ_EP0_IN, length %u\n", pkt->wLength);

            } else if (pkt->bRequest == REQ_EP1_OUT) {

                // pr_debug("\nReceived request REQ_EP1_OUT. Start EP1 OUT %i", length);
                // uint8_t command = buf[0];
                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP1_OUT_ADDR);
                int length = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8);

                usb_init_transfer(ep, length);

            } else if (pkt->bRequest == REQ_EP2_IN) {

                pr_debug("Received request REQ_EP2_IN. Start EP2 IN %i\n", length);

                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP2_IN_ADDR);
                int length = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8);
                struct udd_data *data = &g_udd_data;

                data->cmd = buf[0];
                usb_init_transfer(ep, length);

            } else if (pkt->bRequest == REQ_EP3_OUT) {

                uint8_t command = buf[0];
                int length = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8);
                pr_debug("Received request REQ_EP3_IN. Start EP3 IN %i\n", length);
                pr_debug("%s, command : 0x%02x\n", __func__, command);
                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP3_OUT_ADDR);
                usb_init_transfer(ep, length);

            } else if (pkt->bRequest == REQ_EP4_IN) {

                pr_debug("Received request REQ_EP4_IN. Start EP4 IN\n");
                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP4_IN_ADDR);
                usb_init_transfer(ep, ep->descriptor->wMaxPacketSize);

            }
        }
        break;
    case STAGE_STATUS:
        break;
    default:
        break;
    }
}

/* UDD image transfer handler */
void ep1_out_handler(uint8_t *buf, uint16_t len) {
    // pr_debug("EP1 OUT received %d bytes from host\n", len);
    // for (uint i = 0; i < len; i++) pr_debug("\n0x%02x ", buf[i]);

    start_time = time_us_32();

    // TODO: currently, only support full refresh
    decoder_drawimg(0, 0, buf, len);

    end_time = time_us_32();
}

/* UDD protocal parameter query handler */
void ep2_in_handler(uint8_t *buf, uint16_t len) {
    struct udd_data *data = &g_udd_data;

    pr_debug("EP2 IN sent %d bytes to host\n", len);
    for (uint i = 0; i < len; i++) printf("0x%02x \n", buf[i]);
    printf("%s, command : 0x%02x, len: %d\n", __func__, buf[0], len);

    switch (data->cmd) {
        case 0x00:
            pr_debug("Nop\n");
            buf[0] = 0xEF;
            break;
        case 0x01:
            pr_debug("Read device name\n");
            buf[0] = 0x49;
            buf[1] = 0x4A;
            break;
        case 0x02:
            pr_debug("Read device resolution\n");
            buf[0] = 0x13;
            buf[1] = 0x14;
            break;
        default:
            break;
    }
}

/* UDD protocal parameter set handler */
void ep3_out_handler(uint8_t *buf, uint16_t len) {
    pr_debug("EP3 OUT sent %d bytes to host\n", len);
    for (uint i = 0; i < len; i++) pr_debug("\n0x%02x ", buf[i]);
}

/* UDD touch event report handler */
void ep4_in_handler(uint8_t *buf, uint16_t len) {
    static struct tp_data *tp = &g_udd_data.tp;
    // pr_debug("EP4 IN sent %d bytes to host\n", len);
    // for (uint i = 0; i < len; i++) pr_debug("\n0x%02x ", buf[i]);

    buf[0] = tp->is_pressed;
    buf[1] = tp->x >> 8;
    buf[2] = tp->x & 0xFF;
    buf[3] = tp->y >> 8;
    buf[4] = tp->y & 0xFF;
}
