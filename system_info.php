<h2 id="base-infomation">Base Infomation</h2>
<table border="1">
<?php
        foreach ($MiniOS->configs["system_info"] as $key => $value) {
            echo "<tr>";
            echo "  <td>".$key.":</td>\n";
            $ret = exec($value["cmd"][$MiniOS->system_type]);
            echo "  <td>".$ret."</td>\n";
            echo "  <td>";
            if ($ret != null)
	            echo "   <img src='img/ok.png' width='30' height='30'/>";
            else
	            echo "   <img src='img/error.png' width='30' height='30'/>";
            echo "  </td>";
            echo "</tr>";
        }
?>
</table>
<hr/>
