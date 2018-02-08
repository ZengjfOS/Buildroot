#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016 - zengjf <zengjf42@163.com>

from Config.Configures import config
from logging import *
from WebSocket.WebSocketServer import WSS

def main():
    wss = WSS(config.config["websocketserver"]["wss_address"], config.config["websocketserver"]["wss_port"]);
    wss.start()

if __name__ == '__main__':
    main()
