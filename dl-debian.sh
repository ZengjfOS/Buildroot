#!/bin/sh

export ARCH=arm
export CROSS_COMPILE=~/toolchain/bin/arm-linux-gnueabihf-

#make distclean
make clean
make mx6dlsabresd_defconfig
make u-boot.imx

echo "copy image (:u-boot-imx6dlsabresd_sd.imx"
cp u-boot.imx /media/sf_share/7112/IMX6_L4.1.15_2.0.0_MFG-TOOL/Profiles/Linux/OS\ Firmware/files/u-boot-imx6dlsabresd_sd.imx
sync
sleep 1
echo "sync(:"
date

