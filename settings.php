<?php header("Access-Control-Allow-Origin: *") ?>
<?php
    include 'functions.php';

    $data = json_decode(file_get_contents('php://input'), true);
    $categories = $data["categories"];
    $type = $data["type"];

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

    if ($categories == "hardware_test") {
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

        $result = exec("ping -c 1 -W 1 ".$MiniOS->configs["hardware_test"]["network2"]["value"]." 2>&1 | grep ', 0% packet loss'");
        $json_array["network2_result"] = $result;
        if ($result != null)
            $json_array["data"]["network2"]["status"] = "ok";
        else
            $json_array["data"]["network2"]["status"] = "error";

        $json_array["data"]["network2"]["name"] = "network2";
        $json_array["data"]["network2"]["index"] = 1;
        $json_array["data"]["network2"]["descriptor"] = "network";
        $json_array["data"]["network2"]["range"] = "";
        $json_array["data"]["network2"]["value"] = "";

        $json_array["status"] = "ok";
        echo json_encode($json_array);
    }
?>


