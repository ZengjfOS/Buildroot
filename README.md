# README

## 框架设计思路

* 整个网页内容分为硬件接口、协议测试、应用软件、关于AplexOS；
* 系统层面导航栏采用JSON进行全局配置，导航栏下拉每一行表示一个组件：[js/aplex/aplex_config.js](js/aplex/aplex_config.js)；
* 组件分为：
  * Linker：URL链接内容；
  * HTML：网页内容；
  * Divider：分割线；
* 组件内容面采用AJAX动态加载页面，并处理框架内容：[js/aplex/aplex_frame.js](js/aplex/aplex_frame.js)；
* 每个组件仅有三部分：
  * config.json：组件配置，尽可能做到只需要修改config.js就能完成组件扩容；
  * index.html：组件框架，组件显示大体架构；
  * index.js：组件后台解析脚本，控制组件操作交互，在这个脚本里去获取config.js；
  * index.css：组件样式；
