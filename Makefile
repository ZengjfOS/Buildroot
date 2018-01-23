CONFIG_FILE=freescale_imx6dlsabresd_defconfig

WEBPAGE_HOME=buildroot/output/target/var/www/
WEBPAGE_BACKEND_HOME=buildroot/output/target/var/www/bin/python
GITHUB_REPO=https://github.com/ZengjfOS/Buildroot
WEBPAGE_BRANCH=7112S_WebTest_Page
WEBPAGE_BACKEND_BRANCH=7112S_WebTest_Backend

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
ifneq ($(wildcard $(WEBPAGE_BACKEND_HOME)),)
	git -C $(WEBPAGE_HOME) pull $(GITHUB_REPO) $(WEBPAGE_BRANCH):$(WEBPAGE_BRANCH)
	git -C $(WEBPAGE_BACKEND_HOME) pull $(GITHUB_REPO) $(WEBPAGE_BACKEND_BRANCH):$(WEBPAGE_BACKEND_BRANCH)
else
	git clone $(GITHUB_REPO) --branch $(WEBPAGE_BRANCH) --single-branch $(WEBPAGE_HOME);
	git clone $(GITHUB_REPO) --branch $(WEBPAGE_BACKEND_BRANCH) --single-branch $(WEBPAGE_BACKEND_HOME);
endif

help:
	@echo "USAGE:"
	@echo "    1. make"
	@echo "    2. make config"
	@echo "        use default config: $(CONFIG_FILE)"
	@echo "    3. make savedefconfig"
	@echo "        save default config: $(CONFIG_FILE)"
	@echo "    4. make clean"
	@echo "    5. make webpage"
	@echo "        clone or pull webpage test from GitHub"
	@echo "    6. make help"
