<?php
	/**
	 * get eeprom file data: dd if=/sys/bus/i2c/devices/3-0050/eeprom of=file
	 * vim open file and execute show hex format: `:%!xxd`
	 * get data use php:
	 *     <?php
	 *     	$eepromfile = fopen("/sys/bus/i2c/devices/3-0050/eeprom", "r+") or die("Unable to open file!");
	 *     	print_r(unpack('C*', fgets($eepromfile)));
	 *     	fclose($eepromfile);
	 *     ?>
 	 */
	if ($_GET["version"] == 3) {
		$bin_str .= pack('C', 0x01);
		$bin_str .= pack('C', 0x06);

		$eepromfile = fopen("/sys/bus/i2c/devices/3-0050/eeprom", "w+") or die("Unable to open file!");
		for ($x=0; $x<6; $x++) {
			$byteval = 0;
			for ($j=0; $j<2; $j++) {
				$hexval = 0;
				$charval = ord($_GET["mac"][2 * $x + $j]);
				if ($charval >= ord('0') && $charval <= ord('9'))
					$hexval = $charval - ord('0');
				if ($charval >= ord('a') && $charval <= ord('z'))
					$hexval = $charval - ord('a') + 10;
				if ($charval >= ord('A') && $charval <= ord('Z'))
					$hexval = $charval - ord('A') + 10;

				if (($j % 2) == 0)
					$hexval *= 16;

				$byteval += $hexval;
			}

			$bin_str .= pack('C', $byteval);
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
