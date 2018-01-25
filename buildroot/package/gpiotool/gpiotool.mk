################################################################################
#
# CANopenSocket
#
################################################################################

GPIOTOOL_VERSION = GPIO_Tool
GPIOTOOL_SITE = https://github.com/ZengjfOS/Buildroot
GPIOTOOL_SITE_METHOD = git
GPIOTOOL_LICENSE = LGPLv2.1
GPIOTOOL_LICENSE_FILES = COPYING
GPIOTOOL_INSTALL_STAGING = YES


define GPIOTOOL_CONFIGURE_CMDS
	echo $(@D)
endef


GPIOTOOL_BASE_ENV = \
    CFLAGS="$(TARGET_CFLAGS) -Wall -g $(GPIOTOOL_INCLUDE_DIRS)" \
	LDFLAGS=" -lrt -pthread"

define GPIOTOOL_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(GPIOTOOL_BASE_ENV) \
        -C $(@D) all
endef

define GPIOTOOL_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/gpiotool $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))


