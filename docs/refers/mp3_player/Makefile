default: configs clean
	cd buildroot  && make
	cp -rv ./buildroot/output/images .

configs: submodule clean-configs
	cp -v hughmp3_defconfig buildroot/configs/
	cp -v customize/post_build.sh buildroot/board/raspberrypi2/
	cp -rv customize/overlayfs buildroot/board/raspberrypi2/rootfs-overlay
	cp -v busybox_config buildroot/package/busybox/mrdobox.config
	chmod +x buildroot/board/raspberrypi2/post_build.sh
	cp -v customize/permissions.mk buildroot/board/raspberrypi2/
	sed -i '/menu\ "O/ a   source\ "package\/wiringPi\/Config\.in"' buildroot/package/Config.in
	cp -rvf customize/package/mpg123 buildroot/package/
	cp -rvf customize/package/wiringPi buildroot/package
	cd buildroot && make hughmp3_defconfig

submodule:
	git submodule init && git submodule update
	

clean:
	rm -rf ./images

clean-configs:
	cd buildroot && git clean -f -d

clean-buildroot:
	cd buildroot && make clean

cleandist: clean clean-configs clean-buildroot
	echo "All gone"
