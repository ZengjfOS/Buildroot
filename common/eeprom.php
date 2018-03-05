<?php
	if ($_GET["version"] == 3) {
		$bin_str .= pack('C', 0x01);
		$bin_str .= pack('C', 0x06);

		$eepromfile = fopen("/sys/bus/i2c/devices/3-0050/eeprom", "w+") or die("Unable to open file!");
		for ($x=0; $x<6; $x++) {
			$bin_str .= pack('C', ((int)($_GET["mac"][2 * $x]) * 16 + (int)($_GET["mac"][2 * $x + 1])));
		} 

		for ($x=0; $x<(256 - 8 - 2); $x++) {
			$bin_str .= pack('C', 0x00);
		} 

		$bin_str .= pack('C', 0x03);
		$bin_str .= pack('C', 0x00);

		fwrite($eepromfile, $bin_str);

		fclose($eepromfile);
	}

	echo "OK";
?>
