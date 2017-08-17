<?php
    $output = `bin/hardware_test`;
    $json_array = json_decode($output, true);
    $json_array["zengjf"] = "zengjf";
    echo json_encode($json_array);
?>
