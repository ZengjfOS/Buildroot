# README

## 框架结构

**1. Config/config.json**

* 系统配置文件，配置了所有的系统需要的配置；
* `Config/Configures.py`用于加载`Config/config.json`文件，并保证获取的配置对象是唯一的；
* 需要注意的就是配置中的`categories`分类是用于类映射的，就是字符串创建类的对象的，所以要确保类名对应：
  ```JSON
  "categories": {
    "hardware_gpio": "GPIO",
    "hardware_uart": "UART"
  }
  ```

**2. test**

`test`目录主要是一些测试用的脚本之类的东西
