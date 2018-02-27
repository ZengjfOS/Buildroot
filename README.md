# Buildroot

[https://buildroot.org/](https://buildroot.org/)

**在编译系统的时候遇到有些软件包下载特别慢，可以用浏览器下载，然后放在buildroot根目录的dl目录里面，缩短下载时间。**

dl文件夹包下载：https://pan.baidu.com/s/1i7aOTm9

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
5. `make webpage`  
  Clone or pull webpage test from GitHub
6. `make help`


## Buildroot Hacking Docs

* [docs/README.md](docs/README.md)

## Compile Kernel

* [Source Code](https://github.com/ZengjfOS/Buildroot/tree/fsl_kernel_L4.1.15_from_TP)
* [Compile Shell Script](https://github.com/ZengjfOS/Buildroot/blob/fsl_kernel_L4.1.15_from_TP/remake.sh)

## Compile U-Boot

* [Source Code](https://github.com/ZengjfOS/Buildroot/tree/fsl_uboot_L4.1.15_from_TP)
* [Compile Shell Script](https://github.com/ZengjfOS/Buildroot/blob/fsl_uboot_L4.1.15_from_TP/remake.sh)

## Hardware Test Web Page

只需要修改config文件即可完成自动测试，后台框架采用PHP编码；

* [Web Test](https://github.com/ZengjfOS/Buildroot/tree/7112S_WebTest_Page)

## i.MX6 Download Tool

* [Prebuild U-Boot and Kernel](customize/prebuild/)
* [Download Tool -- MfgTool](https://github.com/ZengjfOS/MfgTool)
