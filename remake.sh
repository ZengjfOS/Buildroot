#!/bin/bash

export PATH=$PATH:/home/zengjf/zengjf/Buildroot/buildroot/output/host/usr/bin/
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
export ARCH=arm

make imx_v7_defconfig

make -j4 LOADADDR=0x10008000 zImage
make -j4 LOADADDR=0x10008000 dtbs

make -j4 LOADADDR=0x10008000 modules 
make -j4 LOADADDR=0x10008000 modules_install INSTALL_MOD_PATH=`pwd`/_install_lib
