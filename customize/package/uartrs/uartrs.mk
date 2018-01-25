################################################################################
#
# CANopenSocket
#
################################################################################

UARTRS_VERSION = fa2aea3857a08d0f0548e72ea60d5ccb5206b7ba
UARTRS_SITE = https://github.com/ZengjfOS/UartRS
UARTRS_SITE_METHOD = git
UARTRS_LICENSE = LGPLv2.1
UARTRS_LICENSE_FILES = COPYING
UARTRS_INSTALL_STAGING = YES


define UARTRS_CONFIGURE_CMDS
	echo $(@D)
endef


UARTRS_BASE_ENV = \
    CFLAGS="$(TARGET_CFLAGS) -Wall -g $(UARTRS_INCLUDE_DIRS)" \
	LDFLAGS=" -lrt -pthread"

define UARTRS_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(UARTRS_BASE_ENV) \
        -C $(@D) all
endef

define UARTRS_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/uartRS $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))


