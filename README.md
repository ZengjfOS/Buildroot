# MiniOS Web Page

该自动化测试框架，只需要配置[configs/system_config.json](configs/system_config.json)文件，无需修改任何其他的内容。

![img/HomePage.png](img/HomePage.png)

## php file include 

* index.php
  * functions.php
  * header.php
  * body.php
    * system_info.php
    * network.php
    * data_and_time.php
    * hardware_test.php

## data protocol

* [docs/json_protocol](docs/json_protocol.md)

## Config Parser

[functions.php](functions.php)

## WebSocket

* [Can Server](https://github.com/Aplexchenfl/cmi_at101_filesystem/blob/master/rootfs_release/usr/share/lighttpd/www/webpages/server.py)
* [JS Client](https://github.com/Aplexchenfl/cmi_at101_filesystem/blob/master/rootfs_release/usr/share/lighttpd/www/webpages/Dido_ctl.php)
