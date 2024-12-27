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
#include "usb.h"

#define TAG "usb: "

#define REQ_EP0_OUT 0X00
#define REQ_EP0_IN 0X01
#define REQ_EP1_OUT 0X02
#define REQ_EP2_IN 0X03

#define DEBUG 1

#if DEBUG
    #define printf(...) printf(TAG __VA_ARGS__)
#else
    #define printf(...)
#endif

uint start_time, end_time;
// uint8_t *ep0_buf, *ep2_buf;

// int usb_init(void) {
//     ep0_buf = usb_get_endpoint_configuration(EP0_OUT_ADDR)->data_buffer;
//     ep2_buf = usb_get_endpoint_configuration(EP2_IN_ADDR)->data_buffer;

//     for (uint i = 0; i < usb_get_endpoint_configuration(EP2_IN_ADDR)->data_buffer_size; i++) {
//         ep2_buf[i] = i;
//     }

//     usb_device_init();

//     while (!usb_is_configured()) {
//     }

//     while (1) {
//     }
// }

void control_transfer_handler(uint8_t *buf, volatile struct usb_setup_packet *pkt, uint8_t stage) {
    // printf("\nControl transfer. Stage %u bmRequestType 0x%x bRequest 0x%x wValue 0x%x wIndex 0x%x wLength %u", stage,
    //        pkt->bmRequestType, pkt->bRequest, pkt->wValue, pkt->wIndex, pkt->wLength);

    // Setup stage.
    if (stage == STAGE_SETUP) {
        if (pkt->bmRequestType == USB_DIR_OUT) {
            if (pkt->bRequest == USB_REQUEST_SET_ADDRESS)
                printf("Set address %u\n", usb_get_address());
            else if (pkt->bRequest == USB_REQUEST_SET_CONFIGURATION)
                printf("Device Enumerated\n");
        }
        else if (pkt->bmRequestType & USB_REQ_TYPE_TYPE_VENDOR) {
            if (pkt->bRequest == REQ_EP0_IN) {
                for (uint i = 0; i < pkt->wLength; i++) buf[i] = i;
            }
        }
    }

    // Data stage
    else if (stage == STAGE_DATA) {
        if (pkt->bmRequestType == USB_DIR_IN) {
            if (pkt->bRequest == USB_REQUEST_GET_DESCRIPTOR) {
                uint16_t descriptor_type = pkt->wValue >> 8;
                switch (descriptor_type) {
                    case USB_DT_DEVICE:
                        printf("Sent device descriptor\n");
                        break;
                    case USB_DT_CONFIG:
                        printf("Sent config descriptor\n");
                        break;
                    case USB_DT_STRING:
                        printf("Sent string descriptor\n");
                        break;
                }
            }
        } else if (pkt->bmRequestType & USB_REQ_TYPE_TYPE_VENDOR) {
            if (pkt->bRequest == REQ_EP0_OUT) {
                printf("Received request REQ_EP0_OUT, length %u\n", pkt->wLength);
                for (uint i = 0; i < pkt->wLength; i++) printf("\n%u ", buf[i]);
                printf("\n");
            } else if (pkt->bRequest == REQ_EP0_IN) {
                printf("Sent request REQ_EP0_IN, length %u\n", pkt->wLength);
            } else if (pkt->bRequest == REQ_EP1_OUT) {
                uint8_t command = buf[0];
                int length = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8);
                // printf("\nReceived request REQ_EP1_OUT. Start EP1 OUT %i", length);
                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP1_OUT_ADDR);
                // printf("\n%s, command : 0x%02x", __func__, command);

                uint8_t *txbuf = ep->data_buffer;
                switch (command) {
                    case 0x51:
                        // printf("\nDraw JPG");
                        printf("last frame: %u us, fps %u\n", end_time - start_time, 1000000 / (end_time - start_time));
                        break;
                    default:
                        break;
                }

                usb_init_transfer(ep, length);
            } else if (pkt->bRequest == REQ_EP2_IN) {
                uint8_t command = buf[0];
                int length = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8);
                printf("Received request REQ_EP2_IN. Start EP2 IN %i\n", length);
                printf("%s, command : 0x%02x\n", __func__, command);

                struct usb_endpoint_configuration *ep = usb_get_endpoint_configuration(EP2_IN_ADDR);
                uint8_t *txbuf = ep->data_buffer;
                switch (command) {
                    case 0x00:
                        printf("Nop\n");
                        txbuf[0] = 0xEF;
                        break;
                    case 0x01:
                        printf("Read device name\n");
                        txbuf[0] = 0x49;
                        txbuf[1] = 0x4A;
                        break;
                    case 0x02:
                        printf("Read device resolution\n");
                        txbuf[0] = 0x13;
                        txbuf[1] = 0x14;
                        break;
                    default:
                        break;
                }
                usb_init_transfer(ep, length);
            }
        }
    }
}

#include "../tjpgd/tjpgd.h"
extern JRESULT jd_getsize(uint16_t *w, uint16_t *h, const uint8_t jpeg_data[], uint32_t  data_size);
extern JRESULT jd_drawjpg(int32_t x, int32_t y, const uint8_t jpeg_data[], uint32_t  data_size);
void ep1_out_handler(uint8_t *buf, uint16_t len) {
    // printf("EP1 OUT received %d bytes from host\n", len);
    // for (uint i = 0; i < len; i++) printf("\n0x%02x ", buf[i]);

    // TODO: currently, only support full refresh
    start_time = time_us_32();

#if 0
    uint16_t w, h;
    JRESULT jresult = JDR_OK;
    jresult = jd_getsize(&w, &h, buf, len);
    if (jresult == JDR_OK)
        jd_drawjpg(0, 0, buf, len);
#else
    jd_drawjpg(0, 0, buf, len);
#endif

    end_time = time_us_32();
}

void ep2_in_handler(uint8_t *buf, uint16_t len) {
    // printf("EP2 IN sent %d bytes to host\n", len);
    // for (uint i = 0; i < len; i++) printf("\n%u ", buf[i]);
}
