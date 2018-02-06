<?php header("Access-Control-Allow-Origin: *") ?>
<?php
    include 'functions.php';

    $data = json_decode(file_get_contents('php://input'), true);
    $categories = $data["categories"];
    $type = $data["type"];

    if ($categories == "hardware_gpio") {

        if ($type == "output") {
            $ret = exec("gpiotool -p mio -m o -i ".$data["index"]." -v ".$data["value"]." | grep -E '^(0|1|-1)'");
            if ($ret == "0") {
                $json_array["status"] = "ok";
            } else{
                $json_array["status"] = "error";
            }
            $json_array["result"] = $ret;
        } else if ($type == "input") {
            for ($i = 0; $i < 4; $i++) {
                $ret = exec("gpiotool -p mio -m i -i ".($i + 4)." | grep -E '^(0|1|-1)'");
                if ($ret == "0" || $ret == "1") {
                    $json_array["gpios"]["in_".$i]["status"] = "ok";
                } else {
                    $json_array["gpios"]["in_".$i]["status"] = "error";
                }
                $json_array["gpios"]["in_".$i]["value"] = $ret;
            } 
            $json_array["status"] = "ok";
        } else {
            $json_array["status"] = "error";
        }


        echo json_encode($json_array);
    }
?>
