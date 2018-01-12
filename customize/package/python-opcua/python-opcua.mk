################################################################################
#
# python-opcua
#
################################################################################

PYTHON_OPCUA_VERSION = e483e47eec93909b920fdff5c1aa5434428a13da
PYTHON_OPCUA_SITE = https://github.com/FreeOpcUa/python-opcua
PYTHON_OPCUA_SITE_METHOD = git
PYTHON_OPCUA_LICENSE = LGPLv2.1
PYTHON_OPCUA_LICENSE_FILES = COPYING
PYTHON_OPCUA_INSTALL_STAGING = YES
PYTHON_OPCUA_SETUP_TYPE = setuptools


$(eval $(python-package))
