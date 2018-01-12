#!/bin/bash

export CROSS_COMPILE=arm-AplexOS-linux-gnueabi-
export ARCH=arm

make mx6dlsabresd_defconfig

make -j4
