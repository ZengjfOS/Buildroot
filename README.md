# README

[https://buildroot.org/](https://buildroot.org/)

**在编译系统的时候遇到有些软件包下载特别慢，可以用浏览器下载，然后放在buildroot根目录的dl目录里面，缩短下载时间。**  

dl文件夹包下载：https://pan.baidu.com/s/1jKh8tka

## Refers

* [Web版用户手册](https://buildroot.org/downloads/manual/manual.html)
* [hugh712/mp3_player](https://github.com/hugh712/mp3_player)
  * [Makefile](https://github.com/hugh712/mp3_player/blob/master/Makefile)
* [buildroot-2017.02.3](docs/refers/buildroot-2017.02.3/README.md)

## USAGE:

1. `make`  
  You need to execute `make config` at first.
2. `make config`  
  Use default config: freescale_imx6dlsabresd_defconfig
3. `make savedefconfig`  
  Save default config: freescale_imx6dlsabresd_defconfig
4. `make clean`
5. `make help`

## Buildroot Hacking Docs

* [docs/README.md](docs/README.md)

## Compile Kernel

Download: https://github.com/ZengjfOS/Buildroot/tree/fsl_uboot_L4.1.15_from_TP

```bash
#!/bin/bash

export CROSS_COMPILE=arm-buildroot-linux-gnueabi-
export ARCH=arm

make imx_v7_defconfig

make -j4 LOADADDR=0x10008000 zImage
make -j4 LOADADDR=0x10008000 dtbs

make -j4 LOADADDR=0x10008000 modules
make -j4 LOADADDR=0x10008000 modules_install INSTALL_MOD_PATH=`pwd`/_install_lib
```

## Compile U-Boot

Download: https://github.com/ZengjfOS/Buildroot/tree/fsl_uboot_L4.1.15_from_TP

```bash
#!/bin/bash

export CROSS_COMPILE=arm-buildroot-linux-gnueabi-
export ARCH=arm

make mx6dlsabresd_defconfig

make -j4
```
