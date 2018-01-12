default:
	cd buildroot && make -j4

config:
	cp -v customize/configs/ZengjfOS_defconfig buildroot/configs/
	cd buildroot && make ZengjfOS_defconfig

savedefconfig:
	cd buildroot && make savedefconfig
	cp -v buildroot/configs/ZengjfOS_defconfig customize/configs/ZengjfOS_defconfig 

clean:
	cd buildroot && make clean

help:
	@echo "USAGE:"
	@echo "    1. make"
	@echo "    2. make config"
	@echo "        use default config: ZengjfOS_defconfig"
	@echo "    3. make savedefconfig"
	@echo "        save default config: ZengjfOS_defconfig"
	@echo "    4. make clean"
	@echo "    5. make help"
