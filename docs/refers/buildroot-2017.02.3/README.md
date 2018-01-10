## README

## Buildroot Package

[buildroot-2017.02.3](buildroot-2017.02.3.tar.bz2)

## Config

* [configs/support_qt5.6_defconfig](configs/support_qt5.6_defconfig)  
  支持Qt 5.6图形库，程序默认是不启动的，需要运行对应的应用程序才能正常的显示界面：
  * [Buildroot Qt 5](http://www.cnblogs.com/zengjfgit/p/7890083.html)
  * [Fluxbox 添加Qt应用程序menu](http://www.cnblogs.com/zengjfgit/p/7910807.html)
* [configs/support_X11_defconfig](configs/support_X11_defconfig)  
  支持X11功能，需要窗口管理器才能显示的比较完善：
  * [X11 fluxbox窗口管理器](http://www.cnblogs.com/zengjfgit/p/7908986.html)
* [configs/support_Qt5_X11_OpcUa_defconfig](configs/support_Qt5_X11_OpcUa_defconfig)  
  Buildroot支持Opc Ua协议：
  * [OpcUaDocs](https://github.com/ZengjfOS/OpcUaDocs)

## Save Buildroot Config

* [Buildroot Savedefconfig](http://www.cnblogs.com/zengjfgit/p/8251499.html)
* [Buildroot 使用默认配置](http://www.cnblogs.com/zengjfgit/p/7461963.html)
* `make list-defconfigs`
  ```
  zengjf@zengjf:~/zengjf/zengjfos/buildroot-2017.02.3$ make list-defconfigs
  Built-in configs:
    acmesystems_aria_g25_128mb_defconfig - Build for acmesystems_aria_g25_128mb
    acmesystems_aria_g25_256mb_defconfig - Build for acmesystems_aria_g25_256mb
    ...
    zynq_zed_defconfig                  - Build for zynq_zed
    zynq_zybo_defconfig                 - Build for zynq_zybo
  ```

## Recompile Package

* [buildroot 重新编译 package](http://www.cnblogs.com/zengjfgit/p/6290719.html)
