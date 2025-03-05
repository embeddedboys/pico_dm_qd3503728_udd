#!/usr/bin/env python3

#
# Copyright (c) 2024 embeddedboys developers
#
# Copyright (c) 2020 Raspberry Pi (Trading) Ltd. author of https://github.com/raspberrypi/pico-examples/tree/master/usb
#
# SPDX-License-Identifier: BSD-3-Clause
#

# sudo pip3 install pyusb

import os
import sys
import cv2

import usb.core
import usb.util
import datetime

# Target resolution for scaling
TARGET_WIDTH = 480
TARGET_HEIGHT = 320

# JPEG quality for compression (1 to 100, higher is better quality)
JPEG_QUALITY = 85

EP_DIR_OUT = 0x00
EP_DIR_IN = 0X80
TYPE_VENDOR = 0X40

REQ_EP0_OUT = 0X00
REQ_EP0_IN = 0X01
REQ_EP1_OUT = 0X02
REQ_EP2_IN = 0X03

def main():
    if len(sys.argv) < 2:
        print("Usage: sudo {} <file.jpg>".format(sys.argv[0]))
        sys.exit(1)

    img = cv2.imread(sys.argv[1])
    height, width, channels = img.shape
    print("Raw image size: {}x{}".format(width, height))

    if width >= height:
        TARGET_WIDTH = 480
        TARGET_HEIGHT = 320
        img = cv2.resize(img, (TARGET_WIDTH, TARGET_HEIGHT))
    else:
        TARGET_WIDTH = 320
        TARGET_HEIGHT = 480
        img = cv2.resize(img, (TARGET_WIDTH, TARGET_HEIGHT))
        img = cv2.rotate(img, cv2.ROTATE_90_COUNTERCLOCKWISE)

    cv2.imwrite("/tmp/.preview.jpg", img, [
        cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY,
        cv2.IMWRITE_JPEG_OPTIMIZE, 1,
    ])

    with open("/tmp/.preview.jpg", "rb") as f:
        pic = f.read()
        if len(pic) % 2 != 0:
            pic = pic + b'\xff'
        buffer = [0x51, len(pic) & 0xFF, len(pic) >> 8, 0x00]
        dev = usb.core.find(idVendor=0x2E8A, idProduct=0x0001)
        if dev is None:
            raise ValueError('Device not found')
        dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP1_OUT, 0, 0, buffer)
        a = datetime.datetime.now()
        dev.write(0x01, pic)
        b = datetime.datetime.now()
        c = b - a
        print("frame took {} ms".format(c.microseconds / 1000))

if __name__ == "__main__":
    main()
