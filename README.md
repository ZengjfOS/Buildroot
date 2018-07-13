# README

WebSocket的代码存放于：[CommonWebSocket branch](https://github.com/ZengjfOS/Buildroot/tree/CommonWebSocket)，使用PyCharm IDE开发。使用WebSocket主要因为像UART、CAN等功能需要用到实时通信才能比较完善的实现。

[MAC Burning Tool](https://github.com/ZengjfOS/BurningMac)

## 框架设计思路

* 整个网页内容分为硬件接口、协议测试、应用软件、关于AplexOS；
* 系统层面导航栏采用JSON进行全局配置，导航栏下拉每一行表示一个组件：[js/aplex/aplex_config.js](js/aplex/aplex_config.js)；
* 组件分为：
  * `Linker`：`URL`链接内容；
  * `HTML`：网页内容；
  * `Divider`：分割线；
* 组件内容面采用AJAX动态加载页面，并处理框架内容：[js/aplex/aplex_frame.js](js/aplex/aplex_frame.js)；
* 每个组件仅有三部分：
  * `config.json`：组件配置，尽可能做到只需要修改`config.js`就能完成组件扩容；
  * `index.html`：组件框架，组件显示大体架构；
  * `index.js`：组件后台解析脚本，控制组件操作交互，在这个脚本里去获取`config.js`；
  * `index.css`：组件样式；
  * `backend.php`：AJAX请求访问后台PHP处理；
  * `functions.php`：PHP需要的预加载处理方法，主要是供`backend.php`使用；

## 框架全局变量

**1. aplexos**

```Javascript
class AplexOS {
    constructor() {
        console.log("AplexOS")
        this.config = null;
        this.ws = null;
    }
}

aplexos = new AplexOS();
```

* `aplexos.config`：可以获取到`js/aplex/aplex_config.js`配置文件内容；
* `aplexos.ws`：可以获取到当前的`websocket`对象，从而重新映射对应的处理函数；


**2. xxx\_init**

每个组件的index.js可以有一个对应的初始化函数，以gpio为例：

```Javascript
// framework callback
function GPIO_init(argv) {
	frame_argv = argv;
	console.log(argv);

	aplexos.ws.onmessage = function(e) {
		ret = JSON.parse(e.data);
		if (ret["type"] == "output" ) {
			if (ret["status"] == "ok") {
				img = $('img[name="output_' + ret["index"] + '"]')[0];
				var fileName = getFileName(img.src);
				if (fileName == "power_gray.png")
					img.src = "img/power_blue.png";
				else
					img.src = "img/power_gray.png";
			}
		}

		if (ret["type"] == "input" ) {
			for (var i = 0; i < color_array.length; i++) {
				if (ret["gpios"] != undefined) {
					img = document.getElementsByName("module1_led_" + color_array[i]);
					var fileName = getFileName(img[0].src);
					var imgName = img[0].name;
				
					if (ret["gpios"]["in_" + i]["status"] == "ok" && ret["gpios"]["in_" + i]["value"] == "0") {
						img[0].src = "img/led_gray.png"
					} else {
						var moduleColor = imgName.substring(imgName.lastIndexOf("_") + 1);	
						img[0].src = "img/led_" + moduleColor + ".png";
					}
				}
			}
		}
	}
}
```

* 可以作为初始化`websocket`的部分；
* 一些变量赋值；
* 函数名是`js/aplex_config`中的`parts`对应的`pages`的`key`名称，参数`argv`中有当前组件的访问路径； 
* 如果这个函数未实现，是不会被调用的：
  ```Javascript
  function call_string_function(function_name, json_data) {
      // string as a function call
      var fn = window[function_name]; 
      if(typeof fn === 'function') 
          fn(json_data);
  }
  
  // dynamic get javascript and run the demo_name function in script file.
  jQuery.loadScript = function (url, callback) {
      jQuery.ajax({
          url: url,
          dataType: 'script',
          success: callback,
          async: true
      });
  }
  function dynamic_get_script(demo_name, demo_js, json_data) {
  
      if (typeof someObject == 'undefined') $.loadScript(demo_js, function(result){
          //Stuff to do after someScript has loaded
          // console.log(demo_name);
          // console.log(json_data);
          call_string_function(demo_name + '_init', json_data);
      });
  }
  ```

**3. index.css使用方法**

* 不要直接修改标准的element来设置样式；
* 使用class重新定义样式；
* 样式前缀最好能够统一，这样可以避免不同的组件相互影响:
  ```CSS
  .sps_header_css{
    height: 56px;
    background-image: url("../img/gateway-01.png");
    background-repeat: repeat-x;
  }
  
  .sps_foot_css{
    height: 34px;
    background-image: url("../img/gateway-03.png");
    background-repeat: repeat-x;
  }
  
  .sps_logo_css{
    float: right;
  }
  ```

**4. config.json使用方法**

```Javascript
$(function(){ 
    $.getJSON(frame_argv["path"] + "/config.json", function(data) {
        MiniConfig = data;

        console.log(MiniConfig);
        ...
    });
});
```

* 如上所示，是组件使用config.json的方法；
* config.json是由组件自己的index.js进行加载的，所以config.json是否使用有组件决定；
