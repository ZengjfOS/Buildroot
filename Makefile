CONFIG_FILE=freescale_imx6dlsabresd_defconfig
WEBPAGE_HOME="buildroot/output/target/var/www/"

default: config
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

webpage:
ifneq ($(wildcard buildroot/output/target/var/www/bin/python),)
	cd $(WEBPAGE_HOME)&& git pull https://github.com/ZengjfOS/Buildroot 7112S_WebTest_Page:7112S_WebTest_Page; 
	cd $(WEBPAGE_HOME)/bin/python && git pull https://github.com/ZengjfOS/Buildroot 7112S_WebTest_Backend:7112S_WebTest_Backend;
else
	git clone https://github.com/ZengjfOS/Buildroot --branch 7112S_WebTest_Page --single-branch $(WEBPAGE_HOME);
	git clone https://github.com/ZengjfOS/Buildroot --branch 7112S_WebTest_Backend --single-branch $(WEBPAGE_HOME)/bin/python;
endif

help:
	@echo "USAGE:"
	@echo "    1. make"
	@echo "    2. make config"
	@echo "        use default config: $(CONFIG_FILE)"
	@echo "    3. make savedefconfig"
	@echo "        save default config: $(CONFIG_FILE)"
	@echo "    4. make clean"
	@echo "    5. make help"
