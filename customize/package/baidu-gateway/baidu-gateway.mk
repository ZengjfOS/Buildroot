################################################################################
#
# baidu-gateway
#
################################################################################

BAIDU_GATEWAY_VERSION = d540774d53645d45b309a1d2f6af2b0c2da7f26c
BAIDU_GATEWAY_SITE = https://github.com/baidu/iot-edge-sdk-for-iot-parser
BAIDU_GATEWAY_SITE_METHOD = git
BAIDU_GATEWAY_LICENSE = LGPLv2.1
BAIDU_GATEWAY_LICENSE_FILES = COPYING
BAIDU_GATEWAY_INSTALL_STAGING = YES


define BAIDU_GATEWAY_CONFIGURE_CMDS
	echo $(@D)
endef


BAIDU_GATEWAY_BASE_ENV = \
    CFLAGS="$(TARGET_CFLAGS) -Wall -g $(BAIDU_GATEWAY_INCLUDE_DIRS)" \
	LDFLAGS=" -lrt -pthread"

ifeq ($(BR2_PACKAGE_BAIDU_GATEWAY_NO_SSL),y)
BAIDU_GATEWAY_BUILD_PATH = /modbus/nossl
endif

ifeq ($(BR2_PACKAGE_BAIDU_GATEWAY_WITH_SSL),y)
BAIDU_GATEWAY_BUILD_PATH = /modbus/withssl
endif

define BAIDU_GATEWAY_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(BAIDU_GATEWAY_BASE_ENV) \
        -C $(@D)$(BAIDU_GATEWAY_BUILD_PATH) bdModbusGateway 
endef

define BAIDU_GATEWAY_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/bdModbusGateway $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))


