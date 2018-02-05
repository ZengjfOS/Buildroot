<?php header("Access-Control-Allow-Origin: *") ?>
<?php
    include 'functions.php';

    $data = json_decode(file_get_contents('php://input'), true);
    $categories = $data["categories"];
    $type = $data["type"];

    // 处理Network部分的设置内容
    if ($categories == "network") {
        if ($type == "dhcp" || $type == "staticIP") {
            $networkFile = fopen("/etc/network/interfaces", "w+");
            $fileString = "";
            if ($type == "dhcp") {
                $fileString = "auto lo \niface lo inet loopback\n\n#dhcp \nauto eth0\niface eth0 inet dhcp\n";
            } elseif ($type == "staticIP"){
                $fileString = $fileString ."auto lo \niface lo inet loopback\n\n";
                $fileString = $fileString ."auto eth0\niface eth0 inet static\n";
                $fileString = $fileString ."    address ".$data["ip"]."\n";
                $fileString = $fileString ."    netmask ".$data["netmask"]."\n";
                $fileString = $fileString ."    broadcast ".$data["broadcast"]."\n";
                $fileString = $fileString ."    gateway ".$data["gateway"]."\n";
            } 
            fwrite($networkFile, $fileString);
            fclose($networkFile);
            
            echo '{"status": "ok"}';
            
            shell_exec('sync');
            shell_exec('reboot');
        }

        if ($type == "ping") {
            $IPOrDNS = $data["IPOrDNS"];
            $result = exec("ping -c 1 '".$IPOrDNS."' 2>&1 | grep '0% packet loss,'");
            if ($result != null)
                echo '{"status": "ok"}';
            else
                echo '{"status": "error"}';
        }
    }

    // 处理时间日期相关的设置内容
    if ($categories == "dateAndTime" && $type == "dateAndTime") {
        $date = $data["date"];
        $time = $data["time"];

        $result = exec("date -s '".$date." ".$time."'");

        if (strpos($result, $time)){
            exec("hwclock -w");
            echo '{"status": "ok"}';
        } else {
            echo '{"status": "error"}';
        }

    }

    // 处理硬件测试部分相关的的内容
    if ($categories == "hardware_test") {
        $test_items = $MiniOS->configs["hardware_test"];
        $test_items_sections = $MiniOS->get_config_sections($test_items);
        foreach ($test_items_sections as $item) {
            if (isset($test_items[$item]["shell"])) {
                $result = exec($test_items[$item]["shell"]);
                $json_array["data"]["hardware_test"][$item]["result"] = $result;
                if ((isset($test_items[$item]["ret"]) && ($result == $test_items[$item]["ret"])) ||
                        (! isset( $test_items[$item]["ret"]) && ($result != null))) {
                    $json_array["data"]["hardware_test"][$item]["status"] = "ok";
                } else {
                    $json_array["data"]["hardware_test"][$item]["status"] = "error";
                }
            }
        }

        foreach ($MiniOS->configs["system_info"] as $key => $value) {
            $ret = exec($value["cmd"][$MiniOS->system_type]);
            $json_array["data"]["system_info"][$key]["result"] = $ret;
            if ($ret != null)
                $json_array["data"]["system_info"][$key]["status"] = "ok";
            else
                $json_array["data"]["system_info"][$key]["status"] = "error";
        }

        $json_array["status"] = "ok";
        echo json_encode($json_array);
    }
?>


