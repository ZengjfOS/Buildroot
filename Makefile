default: configs clean
	cd buildroot && make -j4

configs: clean-configs
	cp -v customize/configs/ZengjfOS_defconfig buildroot/configs/
	cd buildroot && make ZengjfOS_defconfig

clean:
	rm -rf ./images

clean-configs:
	cd buildroot && git clean -f -d

clean-buildroot:
	cd buildroot && make clean

cleandist: clean clean-configs clean-buildroot
	echo "All gone"
