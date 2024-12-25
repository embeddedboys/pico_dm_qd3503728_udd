#!/usr/bin/env python3

#
# Copyright (c) 2020 2024 Daniel Gorbea
#
# Copyright (c) 2020 Raspberry Pi (Trading) Ltd. author of https://github.com/raspberrypi/pico-examples/tree/master/usb
#
# SPDX-License-Identifier: BSD-3-Clause
#

# sudo pip3 install pyusb

import os
import usb.core
import usb.util
import datetime

dev = usb.core.find(idVendor=0x2E8A, idProduct=0x0001)
if dev is None:
    raise ValueError('Device not found')

EP_DIR_OUT = 0x00
EP_DIR_IN = 0X80
TYPE_VENDOR = 0X40

REQ_EP0_OUT = 0X00
REQ_EP0_IN = 0X01
REQ_EP1_OUT = 0X02
REQ_EP2_IN = 0X03

# buffer = [1, 0, 0x00]
# dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP2_IN, 0, 0, buffer)
# response = dev.read(0x82, 1)
# print(response)

# Request EP2
# buffer = [2, 0, 0x01]
buffer = [0x01, 2, 0]
dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP2_IN, 0, 0, buffer)
response = dev.read(0x82, 2)
print(response)

# buffer = [2, 0, 0x02]
# dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP2_IN, 0, 0, buffer)
# response = dev.read(0x82, 2)
# print(response)


contents = []
# content = open("../assets/red.jpg", "rb").read()
# content = list(content)
# contents.append(content)

# content = open("../assets/green.jpg", "rb").read()
# content = list(content)
# contents.append(content)

# ccontent = open("../assets/blue.jpg", "rb").read()
# content = list(content)
# contents.append(ccontent)


# Iterate through the files in the directory
for file_name in sorted(os.listdir("../assets")):
    # Ensure the file matches the expected pattern
    if file_name.startswith("SampleVideo_480x320_") and file_name.endswith(".jpg"):
        file_path = os.path.join("../assets/", file_name)
        try:
            # Read the file in binary mode
            with open(file_path, "rb") as file:
                binary_data = file.read()

            # Check the length of the binary data
            if len(binary_data) <= 40000:
                contents.append(binary_data)
            else:
                print(f"Skipping {file_name}: Binary size exceeds {40000} bytes.")
        except Exception as e:
            print(f"Error reading {file_name}: {e}")


i = 0
while True:
    if i >= len(contents):
        i = 0
    pic = contents[i]
    buffer = [0x51, len(pic) & 0xFF, len(pic) >> 8]
    dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP1_OUT, 0, 0, buffer)
    a = datetime.datetime.now()
    dev.write(0x01, pic)
    b = datetime.datetime.now()
    c = b - a
    fps = 1000000 / c.microseconds;
    print("{:.2f} fps".format(fps))

    # i = (i + 1) % len(contents
    i = i + 1
