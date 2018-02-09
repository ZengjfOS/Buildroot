import configparser
import threading
import os
import pyinotify
import json
import time
from logging import *
from pyinotify import WatchManager, Notifier, ProcessEvent

class InotifyEventHandler(ProcessEvent):

    def process_IN_CLOSE(self, event):
        """
        This method is called on these events: IN_CLOSE_WRITE and
        IN_CLOSE_NOWRITE.
        """
        # reparse the configure file
        if event.name == os.path.basename(Configure.get_config_file()):
            Configure.parse_config()

# 初始化配置
class Configure(threading.Thread):

    __mutex = threading.Lock()
    __configure_file_path = "Config/config.json"

    # 使用单例模式来生成统一的对象
    def __new__(cls, *args, **kwargs):

        if not hasattr(cls, "_inst"):

            # 单例对象
            cls._inst = super(Configure, cls).__new__(cls)
            json_data = open(cls.__configure_file_path)
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

    @classmethod
    def parse_config(cls):

        cls.__mutex.acquire()
        # 配置并解析配置文件
        json_data = open(cls.__configure_file_path)
        cls.config = json.load(json_data)
        cls.__mutex.release()

    @classmethod
    def set_config_file(cls, file_path):

        cls.__configure_file_path = file_path

    @classmethod
    def get_config_file(cls):

        return cls.__configure_file_path

    @classmethod
    def run(cls):
        wm = WatchManager()
        mask = pyinotify.IN_CLOSE_WRITE
        notifier = Notifier(wm, InotifyEventHandler())
        wm.add_watch(os.path.dirname(cls.__configure_file_path), mask, auto_add= True, rec=True)

        # print("now starting monitor config directory." )

        while True:
            try:
                notifier.process_events()
                if notifier.check_events():
                    notifier.read_events()
            except KeyboardInterrupt:
                print("keyboard Interrupt.")
                notifier.stop()
                break

    @classmethod
    def getVal(cls, keys):
        if cls.config != None :
            try:
                cls.__mutex.acquire()
                exec_str = "cls.config"
                for key in keys:
                    exec_str += "[\"" + key + "\"]"
                ret = eval(exec_str)
            except:
                ret = None
            finally:
                cls.__mutex.release()
                return ret
        else :
            return None

config = Configure()

if __name__ == '__main__':

    config.set_config_file("Config/config.json")
    config.start()

