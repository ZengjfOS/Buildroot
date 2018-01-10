# Python3 Config.in Hacking

跟一些Python3的Config.in，不过发现没找到Python模块的配置，所以分析一下Python模块的配置存放在哪里。

## Hakcing [refers/python3/Config.in](refers/python3/Config.in)

```Kconfig
#  /home/aplex/zengjf/zengjfos/buildroot-2017.02.3/.config - Buildroot 2017.02.3 C
# o> Target packages > Interpreter languages and scripting ──────────────────────
#   ┌────────────────── Interpreter languages and scripting ──────────────────┐
#   │  Arrow keys navigate the menu.  <Enter> selects submenus ---> (or empty │  
#   │  submenus ----).  Highlighted letters are hotkeys.  Pressing <Y>        │  
#   │  selectes a feature, while <N> will exclude a feature.  Press           │  
#   │  <Esc><Esc> to exit, <?> for Help, </> for Search.  Legend: [*] feature │  
#   │ ┌────^(-)─────────────────────────────────────────────────────────────┐ │  
#   │ │          External php extensions  --->                              │ │  
#   │ │    [ ] python                                                       │ │  
#   │ │    [*] python3                                                      │ │  
#   │ │          python3 module format to install (.pyc compiled sources onl│ │  
#   │ │          core python3 modules  --->                                 │ │  
#   │ │        External python modules  --->                                │ │  
#   │ │    [ ] ruby                                                         │ │  
#   │ │    [ ] tcl                                                          │ │  
#   │ └─────────────────────────────────────────────────────────────────────┘ │  
#   ├─────────────────────────────────────────────────────────────────────────┤  
#   │        <Select>    < Exit >    < Help >    < Save >    < Load >         │  
#   └─────────────────────────────────────────────────────────────────────────┘  

comment "python3 needs a toolchain w/ wchar, threads, dynamic library"
    depends on BR2_USE_MMU
    depends on !BR2_USE_WCHAR || !BR2_TOOLCHAIN_HAS_THREADS || BR2_STATIC_LIBS

config BR2_PACKAGE_PYTHON3
    bool "python3"
    depends on !BR2_PACKAGE_PYTHON
    depends on BR2_USE_WCHAR
    # uses fork()
    depends on BR2_USE_MMU
    depends on BR2_TOOLCHAIN_HAS_THREADS # libffi
    depends on !BR2_STATIC_LIBS
    select BR2_PACKAGE_LIBFFI
    help
      The python language interpreter.

      http://www.python.org/

if BR2_PACKAGE_PYTHON3

#  /home/aplex/zengjf/zengjfos/buildroot-2017.02.3/.config - Buildroot 2017.02.3 C
# o> Target packages > Interpreter languages and scripting ──────────────────────
# 
#        ┌────────────── python3 module format to install ───────────────┐
#        │  Use the arrow keys to navigate this window or press the      │  
#        │  hotkey of the item you wish to select followed by the <SPACE │  
#        │  BAR>. Press <?> for additional information about this        │  
#        │ ┌───────────────────────────────────────────────────────────┐ │  
#        │ │             ( ) .py sources only                          │ │  
#        │ │             (X) .pyc compiled sources only                │ │  
#        │ │             ( ) .py sources and .pyc compiled             │ │  
#        │ │                                                           │ │  
#        │ │                                                           │ │  
#        │ │                                                           │ │  
#        │ └───────────────────────────────────────────────────────────┘ │  
#        ├───────────────────────────────────────────────────────────────┤  
#        │                    <Select>      < Help >                     │  
#        └───────────────────────────────────────────────────────────────┘  

choice
    prompt "python3 module format to install"
    default BR2_PACKAGE_PYTHON3_PYC_ONLY
    help
      Select Python module format to install on target (py, pyc or both)

config BR2_PACKAGE_PYTHON3_PY_ONLY
    bool ".py sources only"

config BR2_PACKAGE_PYTHON3_PYC_ONLY
    bool ".pyc compiled sources only"

config BR2_PACKAGE_PYTHON3_PY_PYC
    bool ".py sources and .pyc compiled"

endchoice

menu "core python3 modules"

#  /home/aplex/zengjf/zengjfos/buildroot-2017.02.3/.config - Buildroot 2017.02.3 C
# o[...] t packages > Interpreter languages and scripting > core python3 modules
#   ┌───────────────────────── core python3 modules ──────────────────────────┐
#   │  Arrow keys navigate the menu.  <Enter> selects submenus ---> (or empty │  
#   │  submenus ----).  Highlighted letters are hotkeys.  Pressing <Y>        │  
#   │  selectes a feature, while <N> will exclude a feature.  Press           │  
#   │  <Esc><Esc> to exit, <?> for Help, </> for Search.  Legend: [*] feature │  
#   │ ┌─────────────────────────────────────────────────────────────────────┐ │  
#   │ │        *** The following modules are unusual or require extra librar│ │  
#   │ │    [ ] bz2 module                                                   │ │  
#   │ │    [ ] codecscjk module                                             │ │  
#   │ │    [ ] curses module                                                │ │  
#   │ │    [ ] decimal module                                               │ │  
#   │ │    [ ] ossaudiodev module                                           │ │  
#   │ │    [ ] readline                                                     │ │  
#   │ │    -*- ssl                                                          │ │  
#   │ │    [*] unicodedata module                                           │ │  
#   │ │    [ ] sqlite module                                                │ │  
#   │ │    -*- xml module                                                   │ │  
#   │ │    [ ] xz module                                                    │ │  
#   │ │    -*- zlib module                                                  │ │  
#   │ └─────────────────────────────────────────────────────────────────────┘ │  
#   ├─────────────────────────────────────────────────────────────────────────┤  
#   │        <Select>    < Exit >    < Help >    < Save >    < Load >         │  
#   └─────────────────────────────────────────────────────────────────────────┘  

comment "The following modules are unusual or require extra libraries"

config BR2_PACKAGE_PYTHON3_BZIP2
    bool "bz2 module"
    select BR2_PACKAGE_BZIP2
    help
      bzip2 module for Python3

config BR2_PACKAGE_PYTHON3_CODECSCJK
    bool "codecscjk module"
    help
      Chinese/Japanese/Korean codecs module for Python (large).

config BR2_PACKAGE_PYTHON3_CURSES
    bool "curses module"
    select BR2_PACKAGE_NCURSES
    help
      curses module for Python3.

config BR2_PACKAGE_PYTHON3_DECIMAL
    bool "decimal module"
    select BR2_PACKAGE_MPDECIMAL
    help
      decimal module for Python3.

config BR2_PACKAGE_PYTHON3_OSSAUDIODEV
    bool "ossaudiodev module"
    help
      ossaudiodev module for Python3.

config BR2_PACKAGE_PYTHON3_READLINE
    bool "readline"
    select BR2_PACKAGE_READLINE
    help
      readline module for Python3 (required for command-line
      editing in the Python shell).

config BR2_PACKAGE_PYTHON3_SSL
    bool "ssl"
    select BR2_PACKAGE_OPENSSL
    help
      _ssl module for Python3 (required for https in urllib etc).

config BR2_PACKAGE_PYTHON3_UNICODEDATA
    bool "unicodedata module"
    default y
    help
      Unicode character database (used by stringprep module) (large).

config BR2_PACKAGE_PYTHON3_SQLITE
    bool "sqlite module"
    select BR2_PACKAGE_SQLITE
    help
      SQLite database support

config BR2_PACKAGE_PYTHON3_PYEXPAT
    bool "xml module"
    select BR2_PACKAGE_EXPAT
    help
      pyexpat and xml libraries for Python3.

config BR2_PACKAGE_PYTHON3_XZ
    bool "xz module"
    select BR2_PACKAGE_XZ
    help
      xz (a.k.a lzma) module for Python3

config BR2_PACKAGE_PYTHON3_ZLIB
    bool "zlib module"
    select BR2_PACKAGE_ZLIB
    help
      zlib support in Python3

endmenu

endif
```

## Hacking [refers/Config.in](refers/Config.in)


```
menu "Target packages"

    source "package/busybox/Config.in"
    source "package/skeleton/Config.in"

[...省略]

menu "Interpreter languages and scripting"
    source "package/4th/Config.in"
    source "package/enscript/Config.in"
    source "package/python/Config.in"

    [...省略]

    source "package/python3/Config.in"
if BR2_PACKAGE_PYTHON || BR2_PACKAGE_PYTHON3
menu "External python modules"
    source "package/python-alsaaudio/Config.in"
    source "package/python-argh/Config.in"
    source "package/python-arrow/Config.in"
    source "package/python-attrs/Config.in"
    source "package/python-autobahn/Config.in"
    source "package/python-babel/Config.in"
    source "package/python-backports-abc/Config.in"
    source "package/python-beautifulsoup4/Config.in"
    source "package/python-bitstring/Config.in"
    source "package/python-bottle/Config.in"
    source "package/python-can/Config.in"
    source "package/python-cbor/Config.in"
    source "package/python-certifi/Config.in"
    source "package/python-cffi/Config.in"
    source "package/python-characteristic/Config.in"
    source "package/python-chardet/Config.in"
    source "package/python-cheetah/Config.in"
    source "package/python-cherrypy/Config.in"
    source "package/python-click/Config.in"
    source "package/python-coherence/Config.in"
    source "package/python-configobj/Config.in"
    source "package/python-configshell-fb/Config.in"
    source "package/python-constantly/Config.in"
    source "package/python-couchdb/Config.in"
    source "package/python-crc16/Config.in"
    source "package/python-crcmod/Config.in"
    source "package/python-crossbar/Config.in"
    source "package/python-cryptography/Config.in"
    source "package/python-cssselect/Config.in"
    source "package/python-cssutils/Config.in"
    source "package/python-daemon/Config.in"
    source "package/python-dataproperty/Config.in"
    source "package/python-dateutil/Config.in"
    source "package/python-dialog/Config.in"
    source "package/python-dialog3/Config.in"
    source "package/python-dicttoxml/Config.in"
    source "package/python-django/Config.in"
    source "package/python-docopt/Config.in"
    source "package/python-docutils/Config.in"
    source "package/python-dominate/Config.in"
    source "package/python-dpkt/Config.in"
    source "package/python-ecdsa/Config.in"
    source "package/python-engineio/Config.in"
    source "package/python-enum/Config.in"
    source "package/python-enum34/Config.in"
    source "package/python-flask/Config.in"
    source "package/python-flask-babel/Config.in"
    source "package/python-flask-jsonrpc/Config.in"
    source "package/python-flask-login/Config.in"
    source "package/python-flup/Config.in"
    source "package/python-futures/Config.in"
    source "package/python-gobject/Config.in"
    source "package/python-gunicorn/Config.in"
    source "package/python-html5lib/Config.in"
    source "package/python-httplib2/Config.in"
    source "package/python-humanize/Config.in"
    source "package/python-id3/Config.in"
    source "package/python-idna/Config.in"
    source "package/python-incremental/Config.in"
    source "package/python-iniparse/Config.in"
    source "package/python-iowait/Config.in"
    source "package/python-ipaddr/Config.in"
    source "package/python-ipaddress/Config.in"
    source "package/python-ipy/Config.in"
    source "package/python-ipython/Config.in"
    source "package/python-itsdangerous/Config.in"
    source "package/python-jinja2/Config.in"
    source "package/python-jsonschema/Config.in"
    source "package/python-json-schema-validator/Config.in"
    source "package/python-keyring/Config.in"
    source "package/python-libconfig/Config.in"
    source "package/python-lmdb/Config.in"
    source "package/python-logbook/Config.in"
    source "package/python-lxml/Config.in"
    source "package/python-mad/Config.in"
    source "package/python-mako/Config.in"
    source "package/python-markdown/Config.in"
    source "package/python-markdown2/Config.in"
    source "package/python-markupsafe/Config.in"
    source "package/python-mbstrdecoder/Config.in"
    source "package/python-meld3/Config.in"
    source "package/python-mistune/Config.in"
    source "package/python-msgpack/Config.in"
    source "package/python-mutagen/Config.in"
    source "package/python-mwclient/Config.in"
    source "package/python-mwscrape/Config.in"
    source "package/python-mwscrape2slob/Config.in"
    source "package/python-netaddr/Config.in"
    source "package/python-netifaces/Config.in"
    source "package/python-networkmanager/Config.in"
    source "package/python-nfc/Config.in"
    source "package/python-numpy/Config.in"
    source "package/python-paho-mqtt/Config.in"
    source "package/python-pam/Config.in"
    source "package/python-paramiko/Config.in"
    source "package/python-pathpy/Config.in"
    source "package/python-pathtools/Config.in"
    source "package/python-pathvalidate/Config.in"
    source "package/python-pexpect/Config.in"
    source "package/python-pillow/Config.in"
    source "package/python-posix-ipc/Config.in"
    source "package/python-prompt-toolkit/Config.in"
    source "package/python-protobuf/Config.in"
    source "package/python-psutil/Config.in"
    source "package/python-ptyprocess/Config.in"
    source "package/python-pudb/Config.in"
    source "package/python-pyasn/Config.in"
    source "package/python-pyasn-modules/Config.in"
    source "package/python-pycli/Config.in"
    source "package/python-pycparser/Config.in"
    source "package/python-pycrypto/Config.in"
    source "package/python-pydal/Config.in"
    source "package/python-pyelftools/Config.in"
    source "package/python-pyftpdlib/Config.in"
    source "package/python-pygame/Config.in"
    source "package/python-pygments/Config.in"
    source "package/python-pyicu/Config.in"
    source "package/python-pyinotify/Config.in"
    source "package/python-pylru/Config.in"
    source "package/python-pymysql/Config.in"
    source "package/python-pynacl/Config.in"
    source "package/python-pyopenssl/Config.in"
    source "package/python-pyparsing/Config.in"
    source "package/python-pyparted/Config.in"
    source "package/python-pypcap/Config.in"
    source "package/python-pyqrcode/Config.in"
    source "package/python-pyqt/Config.in"
    source "package/python-pyqt5/Config.in"
    source "package/python-pyratemp/Config.in"
    source "package/python-pyro/Config.in"
    source "package/python-pyroute2/Config.in"
    source "package/python-pysendfile/Config.in"
    source "package/python-pysmb/Config.in"
    source "package/python-pysnmp/Config.in"
    source "package/python-pysnmp-apps/Config.in"
    source "package/python-pysnmp-mibs/Config.in"
    source "package/python-pysocks/Config.in"
    source "package/python-pytablereader/Config.in"
    source "package/python-pytablewriter/Config.in"
    source "package/python-pytrie/Config.in"
    source "package/python-pytz/Config.in"
    source "package/python-pyudev/Config.in"
    source "package/python-pyusb/Config.in"
    source "package/python-pyxb/Config.in"
    source "package/python-pyyaml/Config.in"
    source "package/python-pyzmq/Config.in"
    source "package/python-requests/Config.in"
    source "package/python-requests-toolbelt/Config.in"
    source "package/python-rpi-gpio/Config.in"
    source "package/python-rtslib-fb/Config.in"
    source "package/python-scapy3k/Config.in"
    source "package/python-sdnotify/Config.in"
    source "package/python-serial/Config.in"
    source "package/python-service-identity/Config.in"
    source "package/python-setproctitle/Config.in"
    source "package/python-setuptools/Config.in"
    source "package/python-sh/Config.in"
    source "package/python-shutilwhich/Config.in"
    source "package/python-simpleaudio/Config.in"
    source "package/python-simplejson/Config.in"
    source "package/python-singledispatch/Config.in"
    source "package/python-sip/Config.in"
    source "package/python-six/Config.in"
    source "package/python-slob/Config.in"
    source "package/python-smbus-cffi/Config.in"
    source "package/python-socketio/Config.in"
    source "package/python-spidev/Config.in"
    source "package/python-thrift/Config.in"
    source "package/python-tomako/Config.in"
    source "package/python-toml/Config.in"
    source "package/python-tornado/Config.in"
    source "package/python-treq/Config.in"
    source "package/python-twisted/Config.in"
    source "package/python-txaio/Config.in"
    source "package/python-u-msgpack/Config.in"
    source "package/python-ubjson/Config.in"
    source "package/python-ujson/Config.in"
    source "package/python-urllib3/Config.in"
    source "package/python-urwid/Config.in"
    source "package/python-versiontools/Config.in"
    source "package/python-watchdog/Config.in"
    source "package/python-wcwidth/Config.in"
    source "package/python-web2py/Config.in"
    source "package/python-webpy/Config.in"
    source "package/python-werkzeug/Config.in"
    source "package/python-whoosh/Config.in"
    source "package/python-ws4py/Config.in"
    source "package/python-wsaccel/Config.in"
    source "package/python-xlrd/Config.in"
    source "package/python-xlsxwriter/Config.in"
    source "package/python-xlutils/Config.in"
    source "package/python-xlwt/Config.in"
    source "package/python-zope-interface/Config.in"
endmenu
endif

endmenu
```
