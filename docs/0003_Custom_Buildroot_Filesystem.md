# Custom Buildroot Filesystem

https://github.com/hugh712/mp3_player

## customizing the generated target filesystem

* adding or overwriting files on the target filesystem (using BR2_ROOTFS_OVERLAY)  
* modifying or deleting files on the target filesystem (using BR2_ROOTFS_POST_BUILD_SCRIPT)
* running arbitrary commands prior to generating the filesystem image (using BR2_ROOTFS_POST_BUILD_SCRIPT)
* setting file permissions and ownership (using BR2_ROOTFS_DEVICE_TABLE)
* adding custom devices nodes (using BR2_ROOTFS_STATIC_DEVICE_TABLE)
* adding custom user accounts (using BR2_ROOTFS_USERS_TABLES)
* running arbitrary commands after generating the filesystem image (using BR2_ROOTFS_POST_IMAGE_SCRIPT)

```
/home/aplex/zengjf/zengjfos/buildroot-2017.02.3/.config - Buildroot 2017.02.3 C
o> System configuration ───────────────────────────────────────────────────────
┌───────────────────────── System configuration ──────────────────────────┐
│  Arrow keys navigate the menu.  <Enter> selects submenus ---> (or empty │  
│  submenus ----).  Highlighted letters are hotkeys.  Pressing <Y>        │  
│  selectes a feature, while <N> will exclude a feature.  Press           │  
│  <Esc><Esc> to exit, <?> for Help, </> for Search.  Legend: [*] feature │  
│ ┌────^(-)─────────────────────────────────────────────────────────────┐ │  
│ │    ()  Generate locale data                                         │ │  
│ │    [*] Install timezone info                                        │ │  
│ │    (default) timezone list                                          │ │  
│ │    (Etc/UTC) default local time                                     │ │  
│ │    ()  Path to the users tables                                     │ │  
│ │    ()  Root filesystem overlay directories       <-------------     │ │  
│ │    ()  Custom scripts to run before creating filesystem images      │ │  
│ │    ()  Custom scripts to run inside the fakeroot environment        │ │  
│ │    ()  Custom scripts to run after creating filesystem images       │ │  
│ └─────────────────────────────────────────────────────────────────────┘ │  
├─────────────────────────────────────────────────────────────────────────┤  
│        <Select>    < Exit >    < Help >    < Save >    < Load >         │  
└─────────────────────────────────────────────────────────────────────────┘  
```

## [refers/mp3_player/hughmp3_defconfig](refers/mp3_player/hughmp3_defconfig)

```
...
BR2_ROOTFS_OVERLAY="board/raspberrypi2/rootfs-overlay"
BR2_ROOTFS_DEVICE_TABLE="board/raspberrypi2/permissions.mk"
BR2_ROOTFS_POST_BUILD_SCRIPT="board/raspberrypi2/post_build.sh"
...
```

## [refers/mp3_player/Makefile](refers/mp3_player/Makefile)
```
...
cp -v customize/post_build.sh buildroot/board/raspberrypi2/
chmod +x buildroot/board/raspberrypi2/post_build.sh
cp -rv customize/overlayfs buildroot/board/raspberrypi2/rootfs-overlay
cp -v customize/permissions.mk buildroot/board/raspberrypi2/
...
```

## [overlay filesystem](refers/mp3_player/customize/overlayfs/)

```
refers/mp3_player/customize/overlayfs/
├── etc
│   └── init.d
│       ├── S30snd
│       ├── S31fs
│       └── S32usb
├── lib
│   └── mdev
│       └── hughmp3
│           └── automount
└── usr
    └── bin
        ├── autoplay_killall.sh
        └── autoplay.sh
```

## [Insert Packages](refers/mp3_player/Makefile)

```
...
sed -i '/menu\ "O/ a   source\ "package\/wiringPi\/Config\.in"' buildroot/package/Config.in
cp -rvf customize/package/mpg123 buildroot/package/
cp -rvf customize/package/wiringPi buildroot/package/
...
```
