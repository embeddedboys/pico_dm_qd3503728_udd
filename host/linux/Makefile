
# local kernel build dir
KERN_DIR:=/lib/modules/$(shell uname -r)/build

# users kernel dir
# KERN_DIR:=/home/user/linux

PLATFORM=local

MODULE_NAME:=udd

all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

test: all
	sudo rmmod $(MODULE_NAME).ko || true
	sudo insmod $(MODULE_NAME).ko || true

obj-m += $(MODULE_NAME).o
ifeq ($(PLATFORM), local)
	$(MODULE_NAME)-y += usb.o jpegenc.o encoder.o fb.o drm.o dma_gem_dma_helper.o input.o
else
	$(MODULE_NAME)-y += usb.o jpegenc.o encoder.o fb.o drm.o input.o
endif
