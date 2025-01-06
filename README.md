# Pico DM QD3503728 UDD

**UDD(USB Display Device)** 这个项目将树莓派Pico模拟成一个USB显示屏，配合PC上的驱动或软件，来提供显示拓展的功能。
你可以在`scripts`目录下找到可用的python脚本示例。

linux驱动目前还在开发中，位于`host/linux`下

## 构建并烧录

### Raspberry Pi Pico (RP2040)
```bash
mkdir build && cd build
cmake .. -G Ninja
ninja
```

烧录方式二选一
```bash
# UF2
cp src/rp2040-freertos-template.uf2 /media/$USER/RPI-PP2

# CMSIS-DAP
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 25000" -c "program src/rp2040-freertos-template.elf verify reset; shutdown;"
```

### Raspberry Pi Pico2 (RP2350)

```bash
mkdir build-pico2 && cd build-pico2
cmake -DPICO_BOARD=pico2 .. -G Ninja
ninja
```
烧录方式二选一
```bash
# UF2
cp src/rp2040-freertos-template.uf2 /media/$USER/RP2350

# CMSIS-DAP
openocd -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 25000" -c "program src/rp2040-freertos-template.elf verify reset; shutdown;"
```

烧录完成后，屏幕将显示一幅默认画面
![img](./assets/screen_480x320.jpg)

## 测试

在完成上述的编译及烧录步骤后，将USB线缆连接至树莓派Pico的USB接口，此时电脑会识别到新的USB设备，
在Ubuntu上可用`lsusb`命令查看
```bash
Bus 001 Device 039: ID 2e8a:0001 embeddedboys USB Display
```

### Python脚本

安装前置Python库
```
sudo pip3 install pyusb
sudo apt install python3-opencv
```

你可能需要调整脚本中的`JPEG_QUALITY`参数来确保正常运行。

1. 测试 JPG 图片显示

```bash
sudo ./scripts/jpg_viewer.py ~/Pictures/pico_dm_yt350s006.jpg
```

2. 测试 MP4 视频播放

```bash
sudo ./scripts/mp4_player.py ~/Downloads/行走的高原大米饭，鼠兔.mp4
```

> 受限于Pico性能，建议您将视频帧率降低至 15 FPS以获得更流畅的效果

### Linux 驱动

编译并安装驱动
```bash
cd host/linux
make test
```
此时您应该能看到屏幕显示出驱动发来的图片。 后续功能正在开发中。

## 开发

### 软件结构

[./notes/UDD.xmind](./notes/UDD.xmind)

### 协议

[./notes/protocal.md](./notes/protocal.md)
