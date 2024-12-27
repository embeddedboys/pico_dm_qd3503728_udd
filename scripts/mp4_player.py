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
import sys
import cv2

import usb.core
import usb.util
import datetime

# Directory to store extracted frames
FRAMES_DIR = "/tmp/udd_frames"

# Maximum length of binary data to be transmitted
MAX_LENGTH = 40000

# JPEG quality for compression (1 to 100, higher is better quality)
JPEG_QUALITY = 50

# Target resolution for scaling
TARGET_WIDTH = 480
TARGET_HEIGHT = 320

EP_DIR_OUT = 0x00
EP_DIR_IN = 0X80
TYPE_VENDOR = 0X40

REQ_EP0_OUT = 0X00
REQ_EP0_IN = 0X01
REQ_EP1_OUT = 0X02
REQ_EP2_IN = 0X03

def extract_frames_from_video(video_path, output_dir):
    """Extracts frames from a video file and saves them as JPG files."""
    try:
        os.makedirs(output_dir, exist_ok=True)
        # cleanup previously extracted frames
        os.system(f"sudo rm -rf {output_dir}/*")
        video_capture = cv2.VideoCapture(video_path)
        frame_count = 0

        # Iterate through the frames
        print("resize frame to {}x{}".format(TARGET_WIDTH, TARGET_HEIGHT))

        print("Extracting frames from input file...")
        while True:
            success, frame = video_capture.read()
            if not success:
                break

            # Resize the frame to the target resolution
            frame = cv2.resize(frame, (TARGET_WIDTH, TARGET_HEIGHT))

            frame_file = os.path.join(output_dir, f"frame_{frame_count:04}.jpg")
            cv2.imwrite(frame_file, frame, [
                cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY,
                cv2.IMWRITE_JPEG_OPTIMIZE, 1,
            ])
            cv2.IMWRITE_JPEG_OPTIMIZE
            frame_count += 1

        video_capture.release()
        print(f"Extracted {frame_count} frames from {video_path}.")
    except Exception as e:
        print(f"Error extracting frames from {video_path}: {e}")

def main():
    video_file = None
    print("Usage: sudo {} [video_file]".format(sys.argv[0]))
    if len(sys.argv) < 2:
        video_file = "../assets/SampleVideo_480x320.mp4"
    else:
        video_file = sys.argv[1]

    # Validate video file existence
    if not os.path.isfile(video_file):
        print(f"Error: The file '{video_file}' does not exist.")
        return

    # Extract frames from the provided video file
    extract_frames_from_video(video_file, FRAMES_DIR)

    # Initialize the list to store binaries
    media_binaries = []

    # Iterate through the extracted frames
    for file_name in sorted(os.listdir(FRAMES_DIR)):
        file_path = os.path.join(FRAMES_DIR, file_name)

        if file_name.endswith(".jpg"):
            try:
                # Read the file in binary mode
                with open(file_path, "rb") as file:
                    binary_data = file.read()

                # Check the length of the binary data
                if len(binary_data) <= MAX_LENGTH:
                    media_binaries.append(binary_data)
                else:
                    print(f"Skipping {file_name}: Binary size exceeds {MAX_LENGTH} bytes.")
            except Exception as e:
                print(f"Error reading {file_name}: {e}")

    # Output the results
    print(f"Number of frames added to the list: {len(media_binaries)}")

    dev = usb.core.find(idVendor=0x2E8A, idProduct=0x0001)
    if dev is None:
        raise ValueError('Device not found')

    i = 0
    while True:
        if i >= len(media_binaries):
            i = 0
        pic = media_binaries[i]

        if len(pic) % 2 != 0:
            pic = pic + b'\xff'
        buffer = [0x51, len(pic) & 0xFF, len(pic) >> 8, 0x00]
        dev.ctrl_transfer(TYPE_VENDOR | EP_DIR_OUT, REQ_EP1_OUT, 0, 0, buffer)
        a = datetime.datetime.now()
        dev.write(0x01, pic)
        b = datetime.datetime.now()
        c = b - a
        fps = 1000000 / c.microseconds;
        print("{:.2f} fps".format(fps))

        i = (i + 1) % len(media_binaries)


if __name__ == "__main__":
    main()
