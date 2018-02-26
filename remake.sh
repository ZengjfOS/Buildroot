#!/bin/bash

export PATH=/home/zengjf/zengjf/Buildroot/buildroot/output/host/usr/bin:$PATH
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
export ARCH=arm

make mx6dlsabresd_defconfig

make -j4
