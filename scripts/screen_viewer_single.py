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

from Xlib import X, display
import numpy as np

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


def get_screen_pixels():
    d = display.Display()
    root = d.screen().root
    geom = root.get_geometry()
    width, height = geom.width, geom.height

    # 获取原始像素数据（X11 默认格式为 BGRA）
    raw_data = root.get_image(0, 0, width, height, X.ZPixmap, 0xffffffff).data

    # 转换为 numpy 数组 (height, width, 4)
    frame = np.frombuffer(raw_data, dtype=np.uint8).reshape((height, width, 4))

    # 提取 RGB 通道（丢弃 Alpha 通道）
    rgb_frame = frame[:, :, :3]  # shape: (height, width, 3)
    return rgb_frame

def main():
    screen_data = get_screen_pixels()
    img = screen_data
    width, height = img.shape[:2]
    print("Raw image size: {}x{}".format(width, height))

    TARGET_WIDTH = 480
    TARGET_HEIGHT = 320
    img = cv2.resize(img, (TARGET_WIDTH, TARGET_HEIGHT))

    _, jpeg_data = cv2.imencode(
        ".jpg",
        img,
        [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY]
    )
    pic = jpeg_data.tobytes()  # 转换为 bytes 对象

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
