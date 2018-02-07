<?php header("Access-Control-Allow-Origin: *") ?>
<?php
    include 'functions.php';

    if ($_FILES["fileToUpload"]["error"] > 0)
    {
        $json_array["status"] = "error";
        $json_array["result"] = "Return Code: " . $_FILES["fileToUpload"]["error"] . "<br />";
    }
    else
    {
        $json_array["name"] = $_FILES["fileToUpload"]["name"];
        $json_array["size"] = $_FILES["fileToUpload"]["size"];

        /*
        if (file_exists("uploads/" . $_FILES["fileToUpload"]["name"]))
        {
            $json_array["status"] = "error";
            $json_array["result"] = $_FILES["fileToUpload"]["name"] . " already exists. ";
        }
         */
        move_uploaded_file($_FILES["fileToUpload"]["tmp_name"],
            "uploads/" . $_FILES["fileToUpload"]["name"]);

        $data = json_decode($_POST["data"], true);
        $categories = $data["categories"];
        $type = $data["type"];

        if ($categories == "upload") {

            $json_array["type"] = $type;
            if ($type == "uboot") {
                $json_array["status"] = "ok";
                $json_array["cmd"] = $MiniOS->configs[$type]["cmd"];
                $json_array["result"] = shell_exec($MiniOS->configs[$type]["cmd"]);
            } else if ($type == "zImage") {
                $json_array["cmd"] = $MiniOS->configs[$type]["cmd"];
                $json_array["result"] = shell_exec($MiniOS->configs[$type]["cmd"]);
                $json_array["status"] = "ok";
            } else if ($type == "dtb") {
                $json_array["cmd"] = $MiniOS->configs[$type]["cmd"];
                $json_array["result"] = shell_exec($MiniOS->configs[$type]["cmd"]);
                $json_array["status"] = "ok";
            } else if ($type == "file") {
                $json_array["cmd"] = $MiniOS->configs[$type]["cmd"];
                $json_array["result"] = shell_exec($MiniOS->configs[$type]["cmd"]);
                $json_array["status"] = "ok";
            } else {
                $json_array["status"] = "error";
            }

        }
    }
    echo json_encode($json_array);
?>
