#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2016 - zengjf <zengjf42@163.com>

from logging import *
import json

# 初始化配置
class Configures(object):

    # 使用单例模式来生成统一的对象
    def __new__(cls, *args, **kwargs):

        if not hasattr(cls, "_inst"):

            # 单例对象
            cls._inst = super(Configures, cls).__new__(cls)

            # 生成两个内部对象
            json_data = open("Config/config.json")
            cls.config = json.load(json_data)

            # 设置log等级
            infoLevel = DEBUG
            if not cls.config["debug"]:
                infoLevel = ERROR

            if not cls.config["console"]:
                basicConfig(level=infoLevel,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                    filename='ws.log',
                    filemode='a'
                )
            else:
                basicConfig(level=infoLevel,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                )

        return cls._inst

config = Configures()

if __name__ == '__main__':

    # 输出信息：
    #     < __main__.Configures object at 0x7f7404fb3240 >
    #     < __main__.Configures object at 0x7f7404fb3240 >
    #     < __main__.Configures object at 0x7f7404fb3240 >
    debug(Configures())
    debug(Configures())
