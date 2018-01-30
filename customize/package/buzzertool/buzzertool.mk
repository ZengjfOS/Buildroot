################################################################################
#
# BUZZERTOOL
#
################################################################################

BUZZERTOOL_VERSION = Buzzer_Tool
BUZZERTOOL_SITE = https://github.com/ZengjfOS/Buildroot
BUZZERTOOL_SITE_METHOD = git
BUZZERTOOL_LICENSE = LGPLv2.1
BUZZERTOOL_LICENSE_FILES = COPYING
BUZZERTOOL_INSTALL_STAGING = YES


define BUZZERTOOL_CONFIGURE_CMDS
	echo $(@D)
endef


BUZZERTOOL_BASE_ENV = \
    CFLAGS="$(TARGET_CFLAGS) -Wall -g $(BUZZERTOOL_INCLUDE_DIRS)" \
	LDFLAGS=" -lrt -pthread"

define BUZZERTOOL_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(BUZZERTOOL_BASE_ENV) \
        -C $(@D) all
endef

define BUZZERTOOL_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/buzzertool $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))


