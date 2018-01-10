################################################################################
#
# wiringPi
#
################################################################################

WIRINGPI_VERSION = b0a60c3302973ca1878d149d61f2f612c8f27fac
WIRINGPI_SITE = git://git.drogon.net/wiringPi
WIRINGPI_SITE_METHOD = git
WIRINGPI_LICENSE = LGPLv2.1
WIRINGPI_LICENSE_FILES = COPYING
WIRINGPI_INSTALL_STAGING = YES


define WIRINGPI_BUILD_CMDS
	cd $(@D)/wiringPi; make
endef

define WIRINGPI_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/wiringPi/*.h $(TARGET_DIR)/usr/include
	$(INSTALL) -m 0755 -D $(@D)/wiringPi/libwiringPi.so.2.32          $(TARGET_DIR)/lib
	#ln -sf $(TARGET_DIR)/lib/libwiringPi.so.2.32 $(TARGET_DIR)/lib/libwiringPi.so	
	cp $(TARGET_DIR)/lib/libwiringPi.so.2.32 $(TARGET_DIR)/lib/libwiringPi.so	 
endef

define WIRINGPI_INSTALL_STAGING_CMDS
	$(INSTALL) -m 0755 -D $(@D)/wiringPi/*.h $(STAGING_DIR)/usr/include
	$(INSTALL) -m 0755 -D $(@D)/wiringPi/libwiringPi.so.2.32          $(STAGING_DIR)/lib
	ln -sf $(STAGING_DIR)/lib/libwiringPi.so.2.32 $(STAGING_DIR)/lib/libwiringPi.so	
endef
$(eval $(generic-package))
