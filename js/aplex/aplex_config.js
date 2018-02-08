var configs = {
    "title" : "AplexOS",
    "author" : "ZoroZeng",
    "github" : "https://github.com/AplexOS",
    "version" : "0.0.1",
    "language" : "en",
    "websocket": {
        "wss_ip": "192.168.1.202",
        "wss_port": "1883"
    },
    "home_page" : {
        "show" : true,
        "Hardware" : {
            "pages" : {
                "AutoTest" : {
                    "index" : "001",
                    "type" : "html"
                }
            }
        }
    },
    "nav" : {
        // 列表的形式体现导航栏有多少项，模板中用来迭代下面的每一项
        "maps" : [
            {"硬件接口": "Hardware"}, 
            {"通信协议": "Protocol"}, 
            {"应用示例": "Application"}, 
            {"设备信息": "About"}
        ],
        "parts" : ["Hardware", "Protocol", "Application", "About"],
        "Hardware" : {
            "pages" : {
                "AutoTest" : {
                    "index" : "001",
                    "type" : "html"
                },
                "Divider" : {
                    "index" : "000",
                    "type" : "divider"
                },
                "GPIO" : {
                    "index" : "002",
                    "type" : "html"
                },
                "UART" : {
                    "index" : "003",
                    "type" : "html"
                },
                "CAN" : {
                    "index" : "004",
                    "type" : "html"
                },
                "USB" : {
                    "index" : "005",
                    "type" : "html"
                }
            }
        },
        "Protocol" : {
            "pages" : {
                "Modbus-RTU" : {
                    "index" : "001",
                    "type" : "html"
                },
                "Modbus-TCP" : {
                    "index" : "002",
                    "type" : "html"
                },
                "CANopen" : {
                    "index" : "003",
                    "type" : "html"
                }
            }
        },
        "Application" : {
            "pages" : {
                "Temperature" : {
                    "index" : "001",
                    "type" : "html"
                }
            }
        },
        "About" : {
            "pages" : {
                "SystemInfo" : {
                    "index" : "001",
                    "type" : "html",
                },
                "UpdateSystem" : {
                    "index" : "002",
                    "type" : "html",
                }
            }
        }
    }
}
