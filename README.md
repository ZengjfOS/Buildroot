# MiniOS Web Page

## php file include 

* index.php
  * functions.php
  * header.php
  * body.php
    * system_info.php
    * network.php
    * data_and_time.php
    * hardware_test.php

## data protocol

* [docs/json_protocol](docs/json_protocol.md)

## Config Parser

cat functions.php

```
<?php

// author: zengjf
// date  : Tue Aug  1 12:28:41 CST 2017

class MiniOS_context {

    // file path for json configure
    public $config_file_path = "configs/system_config.json";

    // json data format
    public $configs = null;

    // 获取系统类型，X86，ARM
    public $system_type = "Linux";

    public function __construct() {

        // parse configure to json data
        $this->configs = json_decode(file_get_contents($this->config_file_path), true);

        // get running system type
        $this->system_type = exec ("uname -m");
    }

    public function get_configs() {
        if($this->configs==null)
            $this->configs = json_decode(file_get_contents($this->config_file_path), true);

        return $this->configs;
    }

    // how to use:
    //    print_r($MiniOS->get_config_section_value("SystemInfo"));
    public function get_config_section_value($key, $configs = null) {
        if ($configs == null) {
            return $this->configs[$key];
        } else {
            return $configs[$key];
        }
    }

    // get all config sections
    public function get_config_sections($key, $configs = null) {
        $sections = array();

        if ($configs == null) {
            foreach ($this->configs as $section => $section_value){
                $sections[] = $section;
            }
        } else {
            foreach ($configs as $section => $section_value){
                $sections[] = $section;
            }
        }

        print_r($sections);
    }
}

$MiniOS = new MiniOS_context;

?>
```

## System Config Analysis

```
{
    /**
     * 导航栏配置代码: cat navigation.php
     * ...
     * foreach ($MiniOS->configs["navigation"] as $key => $value) {
     *     if ($key == "welcome") {
     *         echo "<li class='toctree-l3'><a href='".$value["link"]."'>".$value["content"]."</a></li>\n";
     *     } else {
     *         echo "<li><a class='toctree-l4' href='".$value["link"]."'>".$value["content"]."</a></li>\n";
     *     }
     * }
     * ...
     */
    "navigation" : {
        "welcome" : {
            "link" : "#welcome-to-arm-settings", 
            "content" : "Welcome to ARM-Settings"
        },
        "system_info" : {
            "link" : "#base-infomation", 
            "content" : "Base Infomation"
        },
        "network" : {
            "link" : "#network",
            "content" : "Network"
        },
        "data_and_time" : {
            "link" : "#dateAndTime",
            "content" : "Date & Time"
        },
        "hardware_test" : {
            "link" : "#hardware-test",
            "content" : "Hardware Test"
        }
    },
    /**
     * foreach ($MiniOS->configs["system_info"] as $key => $value) {
     *     echo "<tr>";
     *     echo "  <td>".$key.":</td>\n";
     *     $ret = exec($value["cmd"][$MiniOS->system_type]);
     *     echo "  <td>".$ret."</td>\n";
     *     echo "  <td>";
     *     if ($ret != null)
     * 	        echo "   <img src='img/ok.png' width='30' height='30'/>";
     *     else
     * 	        echo "   <img src='img/error.png' width='30' height='30'/>";
     *     echo "  </td>";
     *     echo "</tr>";
     * }
     */
    "system_info" : {
        "CPU" : {
            "cmd" : {
                "armv7l" : "grep Hardware /proc/cpuinfo | awk -F ':' '{print $2}' | sed 's/ Sabre-SD Board//g'",
                "x86_64" : "cat /proc/cpuinfo | grep 'model name' | tail -n 1 | awk -F ':' '{print $2}' | xargs"
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : ""
            }

        },
        "Memery" : {
            "cmd" : {
                "armv7l" : "grep MemTotal /proc/meminfo | awk -F ':' '{print $2}' | awk -F ' ' '{print $1}'",
                "x86_64" : "grep MemTotal /proc/meminfo | awk -F ':' '{print $2}' | awk -F ' ' '{print $1}'"
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : ""
            }
        },
        "eMMC" : {
            "cmd" : {
                "armv7l" : "fdisk -l | grep mmcblk3: | awk -F ':' '{print $2}' | awk -F ',' '{print $1}'",
                "x86_64" : "fdisk -l | grep /dev/sda | awk -F ':' '{print $2}' | awk -F ',' '{print $1}' | head -n 1 | xargs"
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : ""
            }
        },
        "eMMC_Available" : {
            "cmd" : {
                "armv7l" : "df -h | grep /dev/root | awk -F ' ' '{print $4}'",
                "x86_64" : ""
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : ""
            }
        },
        "Operating_System_Type" : {
            "cmd" : {
                "armv7l" : "uname",
                "x86_64" : "uname"
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : ""
            }
        },
        "Linux_Version" : {
            "cmd" : {
                "armv7l" : "uname -a | awk -F ' ' '{print $3}'",
                "x86_64" : "uname -a | awk -F ' ' '{print $3}'"
            },
            "compare" : {
                "armv7l" : "",
                "x86_64" : "4.8.0-54-generic"
            }
        },
        "MAC1" : {
            "cmd" : {
                "armv7l" : "ifconfig | grep eth0 | awk -F ' ' '{print $5}'",
                "x86_64" : "uname -a | awk -F ' ' '{print $3}'"
            }
        },
        "MAC2" : {
            "cmd" : {
                "armv7l" : "ifconfig | grep eth1 | awk -F ' ' '{print $5}'",
                "x86_64" : "uname -a | awk -F ' ' '{print $3}'"
            }
        }
    },
    "network" : {
        "dhcp" : {
            "check_mode" : {
                "armv7l" : "grep 'iface eth0 inet dhcp' /etc/network/interfaces",
                "x86_64" : ""
            }
        },
        "static" : {
            "check_mode" : {
                "armv7l" : "grep 'iface eth0 inet static' /etc/network/interfaces",
                "x86_64" : ""
            },
            "ip" : {
                "armv7l" : "ifconfig 'eth0' | grep 'inet ' | awk -F ' ' '{print $2}' | awk -F ':' '{print $2}'",
                "x86_64" : ""
            },
            "netmask" : {
                "armv7l" : "ifconfig 'eth0' | grep 'inet ' | awk -F ' ' '{print $4}' | awk -F ':' '{print $2}'",
                "x86_64" : ""
            },
            "broadcast" : {
                "armv7l" : "ifconfig 'eth0' | grep 'inet ' | awk -F ' ' '{print $3}' | awk -F ':' '{print $2}'",
                "x86_64" : ""
            },
            "gateway" : {
                "armv7l" : "route -n | grep UG | head -n  1 | awk -F ' ' '{print $2}'",
                "x86_64" : "route -n | grep UG | head -n  1 | awk -F ' ' '{print $2}'"
            }
        },
        "ping" : {
            "ip" : "127.0.0.1"
        }
    },
    "date_and_time" : {
        "date" : {
            "armv7l" : "date '+%Y-%m-%d'",
            "x86_64" : "date '+%Y-%m-%d'"
        },
        "time" : {
            "armv7l" : "date '+%H:%M'",
            "x86_64" : "date '+%H:%M'"
        }
    },
    /**
     * foreach ($MiniOS->configs["hardware_test"] as $key => $value) {
     *     echo "<tr>";
     *     echo "<th scope='row'>".$value["index"]."</th>";
     *     echo "<td>".$key."</td>";
     *     echo "<td>".$value["descriptor"]."</td>";
     *     echo "<td>".$value["range"]."</td>";
     *     echo "<td id='".$key."_value'>".$value["value"]."</td>";
     *     echo "<td>";
     *     echo "<img src='img/".$value["status"].".png' width='30' height='30' id='".$key."_status'/>";
     *     echo "</td>";
     *     echo "</tr>";
     * }
     *
     * 图片状态status的id=$key."_status"
     */
    "hardware_test" : {
        "network1" : {
            "index" : 1,
            "descriptor" : "eth0",
            "range" : "192.168.2.10",
            "value" : "192.168.2.10",
            "status" : "error"
        },
        "network2" : {
            "index" : 2,
            "descriptor" : "eth1",
            "range" : "192.168.3.200",
            "value" : "192.168.3.200",
            "status" : "error"
        },
        "rtc" : {
            "index" : 3,
            "descriptor" : "i2c",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "keyboard" : {
            "index" : 4,
            "descriptor" : "usb",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "mouse" : {
            "index" : 5,
            "descriptor" : "usb",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "DB9_RS232" : {
            "index" : 6,
            "descriptor" : "air quality",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "CON3_USB" : {
            "index" : 7,
            "descriptor" : "U Disk",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "CON3_I2C" : {
            "index" : 8,
            "descriptor" : "temperature",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "MIO_8_GPIO" : {
            "index" : 9,
            "descriptor" : "gpio",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "MIO_USB" : {
            "index" : 10,
            "descriptor" : "U disk",
            "range" : "",
            "value" : "",
            "status" : "error"
        },
        "EEPROM" : {
            "index" : 11,
            "descriptor" : "i2c",
            "range" : "",
            "value" : "",
            "status_tag_id" :"cn1_i2c",
            "status" : "error"
        }
    }
}
```
