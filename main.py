#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016 - zengjf <zengjf42@163.com>

from Config.Configure import config
from logging import *
from WebSocket.WebSocketServer import WSS

def main():
    wss = WSS(config.getVal(["websocketserver", "wss_address"]), config.getVal(["websocketserver", "wss_port"]));
    wss.start()

if __name__ == '__main__':
    main()
