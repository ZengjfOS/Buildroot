CONFIG_FILE=freescale_imx6dlsabresd_defconfig

default:
	sudo apt-get install texinfo
	cd buildroot && make -j4

config:
	cp -v customize/configs/$(CONFIG_FILE) buildroot/configs/
	cd buildroot && make $(CONFIG_FILE)

savedefconfig:
	cd buildroot && make savedefconfig
	cp -v buildroot/configs/$(CONFIG_FILE) customize/configs/$(CONFIG_FILE) 

clean:
	cd buildroot && make clean

help:
	@echo "USAGE:"
	@echo "    1. make"
	@echo "    2. make config"
	@echo "        use default config: $(CONFIG_FILE)"
	@echo "    3. make savedefconfig"
	@echo "        save default config: $(CONFIG_FILE)"
	@echo "    4. make clean"
	@echo "    5. make help"
