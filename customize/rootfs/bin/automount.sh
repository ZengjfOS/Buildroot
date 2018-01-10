#!/bin/sh

# 挂载路径
MOUNT_PATH=""
# 信息输出串口
MSG_PATH="/dev/ttymxc0"

# 设备节点为空检查，退出
if [ $1 == "" ]; then
    echo "[AutoMount] automount.sh parameter is none" > $MSG_PATH
    exit 0
# eMMC或者SD卡挂载
elif [ ${1:0:6} == "mmcblk" ]; then
    name=$1
    MOUNT_PATH="/mnt/sdcard/"$name
# 其他类型的盘挂载
else
    name=$1
    check=${name%[1-9]}
    vendor=`cat /sys/block/$check/device/vendor`
    # SATA挂载
    if [ $vendor == "ATA" ]; then
       MOUNT_PATH="/mnt/hdd/"$name
    # U盘挂载
    else
       MOUNT_PATH="/mnt/usb/"$name
    fi  
fi
# 将挂载点信息输出到串口
echo "[AutoMount] mount path : $MOUNT_PATH" >> $MSG_PATH

# 获取当前需要挂载、卸载的分区格式
FORMAT=`/sbin/fdisk -l | grep $1`
# 查看是否已经有挂载设备了，如果已经挂载，那当前次就要卸载
OLD_MOUNT_PATH=`mount | grep $MOUNT_PATH`

# 未挂载，那么当前次就是需要进行挂载
if [ "$OLD_MOUNT_PATH" == "" ]; then

    # 如果挂载点不存在，那么就自动创建挂载点
    if [ ! -x $MOUNT_PATH ]; then
        mkdir -p $MOUNT_PATH
    fi 

    # 针对格式进行挂载
    for token in $FORMAT
    do
        if [ $token == "FAT32" ]; then
            mount -t vfat /dev/$1 $MOUNT_PATH
            echo "[AutoMount] mount -t vfat /dev/$1 $MOUNT_PATH" > $MSG_PATH
        elif [ $token == "FAT16" ]; then
            mount -t vfat /dev/$1 $MOUNT_PATH
            echo "[AutoMount] mount -t vfat /dev/$1 $MOUNT_PATH" > $MSG_PATH
        elif [ $token == "HPFS/NTFS" ]; then
            if [ -f /usr/app/ntfs-3g ]; then
                echo "[AutoMount] /usr/app/ntfs-3g /dev/"$1" "$MOUNT_PATH > /var/log/automount/mount.log
                export LD_LIBRARY_PATH=/lib/:${LD_LIBRARY_PATH}
                /usr/app/ntfs-3g /dev/$1 $MOUNT_PATH
                echo "[AutoMount] /usr/app/ntfs-3g /dev/$1 $MOUNT_PATH" > $MSG_PATH
            else
                mount -t ntfs /dev/$1 $MOUNT_PATH
                echo "[AutoMount] mount -t ntfs /dev/$1 $MOUNT_PATH" > $MSG_PATH
            fi
        # 没有针对的格式，直接进行挂载
        elif [ $token == "Linux" ]; then
            mount /dev/$1 $MOUNT_PATH
            echo "[AutoMount] mount /dev/$1 $MOUNT_PATH" > $MSG_PATH
        fi
    done
# 自动卸载
else
    umount $MOUNT_PATH
    echo "[AutoMount] umount "$MOUNT_PATH > $MSG_PATH
fi
