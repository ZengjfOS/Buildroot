################################################################################
#
# CANopenSocket
#
################################################################################

CANOPENSOCKET_VERSION = 6606856c76f5ac13f8db8d22766b078803af736c
CANOPENSOCKET_SITE = https://github.com/CANopenNode/CANopenSocket
CANOPENSOCKET_SITE_METHOD = git
CANOPENSOCKET_LICENSE = LGPLv2.1
CANOPENSOCKET_LICENSE_FILES = COPYING
CANOPENSOCKET_INSTALL_STAGING = YES

CANOPENSOCKET_SRC_PATH = `pwd`/output/build/canopensocket-6606856c76f5ac13f8db8d22766b078803af736c/

define CANOPENSOCKET_CONFIGURE_CMDS
	echo $(CANOPENSOCKET_SRC_PATH)
	mkdir -p $(CANOPENSOCKET_SRC_PATH)/CANopenNode
	tar xvf `pwd`/package/canopensocket/canopennode-dee8503da0c943ced12e482f0025204a2f6dffaf.tar.gz -C $(CANOPENSOCKET_SRC_PATH)/CANopenNode
	mv $(CANOPENSOCKET_SRC_PATH)/CANopenNode/canopennode-dee8503da0c943ced12e482f0025204a2f6dffaf/* $(CANOPENSOCKET_SRC_PATH)/CANopenNode/
endef

CANOPENSOCKET_STACKDRV_SRC =  ../CANopenNode/stack/socketCAN
CANOPENSOCKET_STACK_SRC =     ../CANopenNode/stack
CANOPENSOCKET_CANOPEN_SRC =   ../CANopenNode
CANOPENSOCKET_CANOPEND_SRC =  ./src
CANOPENSOCKET_OBJ_DICT_SRC =  ./objDict
CANOPENSOCKET_APP_SRC =       ./app

CANOPENSOCKET_INCLUDE_DIRS = -I$(CANOPENSOCKET_STACKDRV_SRC) \
                             -I$(CANOPENSOCKET_STACK_SRC)    \
                             -I$(CANOPENSOCKET_CANOPEN_SRC)  \
                             -I$(CANOPENSOCKET_CANOPEND_SRC) \
                             -I$(CANOPENSOCKET_OBJ_DICT_SRC) \
                             -I$(CANOPENSOCKET_APP_SRC)

CANOPENSOCKET_BASE_ENV = \
    CFLAGS="$(TARGET_CFLAGS) -Wall -g $(CANOPENSOCKET_INCLUDE_DIRS)" \
	LDFLAGS=" -lrt -pthread"

define CANOPENSOCKET_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(CANOPENSOCKET_BASE_ENV) \
        -C $(@D)/canopend all
    cp -v $(@D)/canopend/app/canopend $(TARGET_DIR)/usr/bin

    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(CANOPENSOCKET_BASE_ENV) \
        -C $(@D)/canopencomm all
    cp -v $(@D)/canopencomm/canopencomm $(TARGET_DIR)/usr/bin

    $(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) $(CANOPENSOCKET_BASE_ENV) \
        -C $(@D)/canopencgi all
    cp -v $(@D)/canopencgi/canopen.cgi $(TARGET_DIR)/usr/bin
endef

define CANOPENSOCKET_INSTALL_TARGET_CMDS
    # $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install DESTDIR=$(TARGET_DIR)
endef

$(eval $(generic-package))


