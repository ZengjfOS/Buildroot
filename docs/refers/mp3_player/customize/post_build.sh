#!/bin/sh

FS=$TARGET_DIR  # The root filesystem
MDEV_CONF=$TARGET_DIR/etc/mdev.conf
INITTAB=$FS/etc/inittab

# Remove network init script (networking is also disabled in the kernel)
rm -f $FS/etc/init.d/S40network

# Set up automount and autoplay
echo "# Automatically mount and play VFAT volumes" >> $MDEV_CONF
echo "sd[a-z].* root:disk 660 */lib/mdev/hughmp3/automount" >> $MDEV_CONF

# add UART getty
echo "" >>$INITTAB
echo "# UART getty for Ra pi2" >> $INITTAB
echo "ttyAMA0::respawn:/sbin/getty -L  ttyAMA0 115200 vt100 " >>$INITTAB
