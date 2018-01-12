################################################################################
#
# python-websocket-server
#
################################################################################

PYTHON_WEBSOCKET_SERVER_VERSION = 78b222d6e5b5b30d65ef4e49c03f4dec242aaa44
PYTHON_WEBSOCKET_SERVER_SITE = https://github.com/Pithikos/python-websocket-server
PYTHON_WEBSOCKET_SERVER_SITE_METHOD = git
PYTHON_WEBSOCKET_SERVER_LICENSE = LGPLv2.1
PYTHON_WEBSOCKET_SERVER_LICENSE_FILES = COPYING
PYTHON_WEBSOCKET_SERVER_INSTALL_STAGING = YES
PYTHON_WEBSOCKET_SERVER_SETUP_TYPE = setuptools


$(eval $(python-package))
