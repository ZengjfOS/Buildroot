<?php
    if ($_FILES["fileToUpload"]["error"] > 0)
    {
        echo "Return Code: " . $_FILES["fileToUpload"]["error"] . "<br />";
    }
    else
    {
        echo "Upload: " . $_FILES["fileToUpload"]["name"] . "<br />";
        echo "Type: " . $_FILES["fileToUpload"]["type"] . "<br />";
        echo "Size: " . ($_FILES["fileToUpload"]["size"] / 1024) . " Kb<br />";
        echo "Temp file: " . $_FILES["fileToUpload"]["tmp_name"] . "<br />";

        if (file_exists("uploads/" . $_FILES["fileToUpload"]["name"]))
        {
            echo $_FILES["fileToUpload"]["name"] . " already exists. ";
        }
        else
        {
            move_uploaded_file($_FILES["fileToUpload"]["tmp_name"],
            "uploads/" . $_FILES["fileToUpload"]["name"]);
            echo "Stored in: " . "uploads/" . $_FILES["fileToUpload"]["name"];

            $data = json_decode($_POST["data"], true);
            $categories = $data["categories"];
            $type = $data["type"];

            echo "\r\n".var_dump($data);
        }
    }
?>
