default:
	cd buildroot && make -j4

config:
	cp -v customize/configs/freescale_imx6dlsabresd_defconfig buildroot/configs/
	cd buildroot && make freescale_imx6dlsabresd_defconfig

savedefconfig:
	cd buildroot && make savedefconfig
	cp -v buildroot/configs/freescale_imx6dlsabresd_defconfig customize/configs/freescale_imx6dlsabresd_defconfig 

clean:
	cd buildroot && make clean

help:
	@echo "USAGE:"
	@echo "    1. make"
	@echo "    2. make config"
	@echo "        use default config: freescale_imx6dlsabresd_defconfig"
	@echo "    3. make savedefconfig"
	@echo "        save default config: freescale_imx6dlsabresd_defconfig"
	@echo "    4. make clean"
	@echo "    5. make help"
