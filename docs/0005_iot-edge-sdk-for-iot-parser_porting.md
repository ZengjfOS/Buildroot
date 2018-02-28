# iot-edge-sdk-for-iot-parser porting

## Commit URL

https://github.com/ZengjfOS/Buildroot/commit/d4c53625c83315ffeee56571d03077a46f25bb00

## 基本操作

* Buildroot版本：buildroot-2017.02.3.tar.bz2
* 物解析GitHub仓库：https://github.com/baidu/iot-edge-sdk-for-iot-parser
* 参考物解析ARM编译：https://github.com/baidu/iot-edge-sdk-for-iot-parser/blob/master/modbus/cross-compile/linux-arm.sh
* 使用3.1.4版本的libmodbus库：https://github.com/ZengjfOS/Buildroot/blob/master/buildroot/package/libmodbus/libmodbus.mk
* 使用1.5.9版本的cJSON库：https://github.com/ZengjfOS/Buildroot/blob/master/buildroot/package/cjson/cjson.mk
* make menuconfig 选择好cJSON、libmodbus、paho-mqtt-c、OpenSSL库；
* 添加iot-edge-sdk-for-iot-parser支持包：https://github.com/ZengjfOS/Buildroot/tree/master/buildroot/package/baidu-gateway
* make
