# Host-side Drivers

## linux/

A linux USB device driver with Framebuffer and Input device driver implenments.

### Build
```
cd linux
make
```

### Depoly

#### modprobe
```
sudo cp udd.ko /lib/modules/`uname -r`/
sudo depmod -a
echo "udd" > /etc/modules-load.d/udd.conf

sudo modprobe udd
```
#### DKMS

DKMS will automatically build this driver after a new kernel is installed.

## windows/

Help wanted. I'm not a windows driver developer.

## cross-platform python script host
