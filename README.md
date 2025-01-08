# Pico DM QD3503728 UDD

> [!WARNING]
> 本工程正处于早期开发阶段

Pico **UDD(USB Display Device)** 这个项目将树莓派Pico模拟成一个USB显示屏，配合主机端的驱动或软件，来为主机提供显示拓展的功能。 开发本项目的初衷是为一些 linux 板卡提供简单的显示触摸支持。

你可以在`scripts`目录下找到可用的python脚本示例，
linux驱动目前还在开发中，位于`host/linux`下，参见[主机端软件和驱动](#主机端软件和驱动)。

## 待办项

- [ ] 设备端双核工作
- [ ] 更好的编解码过程
- [ ] linux 驱动输入支持
- [ ] linux DRM 驱动

## 特性

- 开箱即用
- 兼容 Pico 和 Pico2 核心板
- 平均 15 FPS 的刷新速率
- 开源驱动

## 构建并烧录设备端固件

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

烧录完成后，显示模组屏幕将显示一幅默认画面, 如下图所示：

![img](./assets/screen_480x320.jpg)

## 主机端软件和驱动

在完成上述设备固件的编译及烧录步骤后，将USB线缆连接至树莓派Pico的USB接口，此时电脑会识别到一个新的USB设备，
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

您可能需要调整脚本中的`JPEG_QUALITY`参数来确保正常运行。

1. 测试 JPG 图片显示

```bash
sudo ./scripts/jpg_viewer.py ~/Pictures/pico_dm_yt350s006.jpg
```

2. 测试 MP4 视频播放

```bash
sudo ./scripts/mp4_player.py ~/Downloads/行走的高原大米饭，鼠兔.mp4
```

> 受限于设备性能，建议您将视频帧率降低至 15 FPS 以获得更流畅的效果

### Linux 驱动

当前的 linux 驱动是基于 fbdev 实现的，对于上层软件的支持性没有 DRM 好，我们后续会更换为 DRM 的版本。

我的测试设备运行的操作系统是 Linux mint 21.3，基于ubuntu jammy，内核版本为 5.15.0-130-generic

#### 安装驱动

将设备连接至PC后，编译并安装驱动
```bash
cd host/linux
make test
```

如果一切正常，可通过`dmesg`命令查看到内核打印的如下日志：
```bash
[11431.830214]

               udd_probe
[11431.830215] udd-fb: udd_framebuffer_alloc
[11431.830216] udd-fb: vmem_size: 307200
[11431.847831] 300 KB video memory
[11431.847856] usbcore: registered new interface driver udd
```

#### fbterm 测试

你可能需要先安装fbterm
```bash
sudo apt install fbterm -y
```

安装完成后，使用 `Ctrl+Alt+F1` 键，进入虚拟终端tty1，使用 `Ctrl+Alt+F7` 返回桌面环境。请记住这两个快捷键之后，再进行后续操作。

假设你已经切换到了tty1，输入如下命令，将 fbterm 映射到`/dev/fb1`上。对于一些没有默认显示设备的 linux 开发板，可能为 `/dev/fb0`
```bash
FRAMEBUFFER=/dev/fb1 fbterm
```

此时您应该可以看到控制台出现在了显示模组上，然后您可以通过键盘输入字符、命令等。

#### lv_port_linux 测试

```bash
# 拉取代码
git clone https://github.com/lvgl/lv_port_linux.git

# 编译
cd lv_port_linux
mkdir build && cd build
ninja

# 运行
LV_LINUX_FBDEV_DEVICE=/dev/fb1 ../bin/lvglsim
```

#### 卸载驱动

在执行其他python脚本前，您应当先卸载此驱动程序

卸载驱动前，请先退出所有正在使用fbdev的应用，然后执行如下命令
```bash
sudo rmmod udd
```

### Windows 驱动

我不是 Windows 驱动开发人员，这可能需要一些时间。。。

## 开发

### 软件结构

[./notes/UDD.xmind](./notes/UDD.xmind)

### 协议

[./notes/protocal.md](./notes/protocal.md)

## 本项目使用的开源软件

- [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)
- [bitbank2/JPEGENC](https://github.com/bitbank2/JPEGENC)
- [dgatf/usb_library_rp2040](https://github.com/dgatf/usb_library_rp2040)
- [Bodmer/TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder)
- [embeddedboys/pico_dm_qd3503728_freertos](https://github.com/embeddedboys/pico_dm_qd3503728_freertos)