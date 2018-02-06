<?php header("Access-Control-Allow-Origin: *") ?>
<?php
    include 'functions.php';

    $data = json_decode(file_get_contents('php://input'), true);
    $categories = $data["categories"];
    $type = $data["type"];

    // 处理硬件测试部分相关的的内容
    if ($categories == "hardware_gpio") {

        if ($type == "output") {
            $json_array["status"] = "ok";
        } else if ($type == "input") {

            $json_array["status"] = "ok";
        } else {
            $json_array["status"] = "error";
        }


        echo json_encode($json_array);
    }
?>


