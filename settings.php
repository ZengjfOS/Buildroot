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
        // 执行硬件测试程序，获取返回的json数据
        $output = `bin/hardware_test`;
        $json_array = json_decode($output, true);

        $result = exec("ping -c 1 -W 1 ".$MiniOS->configs["hardware_test"]["network1"]["value"]." 2>&1 | grep ', 0% packet loss'");
        $json_array["network1_result"] = $result;
        if ($result != null)
            $json_array["data"]["network1"]["status"] = "ok";
        else
            $json_array["data"]["network1"]["status"] = "error";

        $json_array["data"]["network1"]["name"] = "network1";
        $json_array["data"]["network1"]["index"] = 1;
        $json_array["data"]["network1"]["descriptor"] = "network";
        $json_array["data"]["network1"]["range"] = "";
        $json_array["data"]["network1"]["value"] = "";

        $result = exec($MiniOS->configs["hardware_test"]["rtc"]["shell"]);
        $json_array["data"]["rtc"]["result"] = $result;
        if ($result == "0")
            $json_array["data"]["rtc"]["status"] = "ok";
        else
            $json_array["data"]["rtc"]["status"] = "error";
        $json_array["data"]["rtc"]["name"] = "rtc";

        $result = exec($MiniOS->configs["hardware_test"]["keyboard"]["shell"]);
        $json_array["data"]["input"]["keyboard"]["result"] = $result;
        if ($result != null)
            $json_array["data"]["input"]["keyboard"]["status"] = "ok";
        else
            $json_array["data"]["input"]["keyboard"]["status"] = "error";
        $json_array["data"]["input"]["keyboard"]["name"] = "keyboard";

        $result = exec($MiniOS->configs["hardware_test"]["mouse"]["shell"]);
        $json_array["data"]["input"]["mouse"]["result"] = $result;
        if ($result != null)
            $json_array["data"]["input"]["mouse"]["status"] = "ok";
        else
            $json_array["data"]["input"]["mouse"]["status"] = "error";
        $json_array["data"]["input"]["mouse"]["name"] = "mouse";

        $result = exec($MiniOS->configs["hardware_test"]["MIO_USB"]["shell"]);
        $json_array["data"]["udisk"]["result"] = $result;
        if ($result != null)
            $json_array["data"]["udisk"]["status"] = "ok";
        else
            $json_array["data"]["udisk"]["status"] = "error";
        $json_array["data"]["udisk"]["name"] = "MIO_USB";

        $result = exec($MiniOS->configs["hardware_test"]["EEPROM"]["shell"]);
        $json_array["data"]["eeprom"]["result"] = $result;
        if ($result != null)
            $json_array["data"]["eeprom"]["status"] = "ok";
        else
            $json_array["data"]["eeprom"]["status"] = "error";
        $json_array["data"]["eeprom"]["name"] = "EEPROM";

        $result = exec($MiniOS->configs["hardware_test"]["SDCard"]["shell"]);
        $json_array["data"]["SDCard"]["result"] = $result;
        if ($result != null)
            $json_array["data"]["SDCard"]["status"] = "ok";
        else
            $json_array["data"]["SDCard"]["status"] = "error";
        $json_array["data"]["SDCard"]["name"] = "SDCard";

        $result = exec($MiniOS->configs["hardware_test"]["DB9_RS232"]["shell"]);
        $json_array["data"]["DB9_RS232"]["result"] = $result;
        $json_array["data"]["DB9_RS232"]["value"] = $result;
        if ($result != null)
            $json_array["data"]["DB9_RS232"]["status"] = "ok";
        else
            $json_array["data"]["DB9_RS232"]["status"] = "error";
        $json_array["data"]["DB9_RS232"]["name"] = "DB9_RS232";

        $json_array["status"] = "ok";
        echo json_encode($json_array);
    }
?>


