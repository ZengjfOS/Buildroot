# Configuration_Information

## 目录架构


## json configure

```
    var configs = {
        "title" : "Web Drawing",
        "author" : "zengjf",
        "blog" : "http://www.cnblogs.com/zengjfgit/",
        "github" : "https://github.com/ZengjfOS",
        "version" : "0.0.1",
        "nav" : {
            // 列表的形式体现导航栏有多少项，模板中用来迭代下面的每一项
            "parts" : ["SVG", "Canvas", "Three_js", "Show_Time", "My_Girl", "Keep_Walk", "About"],
            "SVG" : {
                "pages" : {
                    /**
                     * 对于type是html类型且需要渲染的的page来说，文件夹的名称是$(index)_$(key)，例如：
                     * 001_javascript_control_svg_element = $(index)(0001)_$(key)(javascript_control_svg_element)
                     */
                    "javascript_control_svg_element" : {
                        "index" : "001",
                        "type" : "html"
                    }
                }
            },
            "Canvas" : {
                "pages" : {
                }
            },
            "Three_js" : {
                "pages" : {
                }
            },
            "Show_Time" : {
                "pages" : {
                    "serial_assistant" : {
                        "index" : "001",
                        "type" : "html"
                    }
                }
            },
            "My_Girl" : {
                "pages" : {
                }
            },
            "Keep_Walk" : {
                "pages" : {
                }
            },
            "About" : {
                "pages" : {
                    /**
                     * type为divider的类型为分隔符
                     */
                    "divider1" : {
                        "type" : "divider",
                    },
                    /**
                     * type为linker的类型为直接的页面链接
                     */
                    "Personal Blog" : {
                        "type" : "linker",
                        "url" : "http://www.cnblogs.com/zengjfgit/"
                    },
                    "GitHub Account" : {
                        "type" : "linker",
                        "url" : "https://github.com/ZengjfOS"
                    },
                    "GitHub Repository" : {
                        "type" : "linker",
                        "url" : "https://github.com/ZengjfOS/ZengjfOS.github.io"
                    },
                    "divider2" : {
                        "type" : "divider",
                    },
                    /**
                     * 对于type是html类型直接html或者Markdown文档，直接用网址
                     */
                    "Website_WolkFlow" : {
                        "type" : "html",
                        "markdown" : "url",
                        "url" : "http://zorozeng.com/docs/001_WebSite_WorkFlow.html"
                    },
                    "Configuration_Information" : {
                        "type" : "html",
                        "markdown" : "url",
                        "url" : "http://zorozeng.com/docs/002_Configuration_Information.md"
                    },
                    "Website_Information" : {
                        "type" : "html",
                        "markdown" : "url",
                        "url" : "http://zorozeng.com/docs/003_Website_Information.md"
                    },
                    "README.md" : {
                        "type" : "html",
                        "markdown" : "url",
                        "url" : "http://zorozeng.com/README.md"
                    }
                }
            }
        }
    }
```
