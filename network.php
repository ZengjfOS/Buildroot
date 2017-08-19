<h2 id="network">Network</h2>
<ul>
  <!-- for data & time -->
  <link rel="stylesheet" type="text/css" href="css/datedropper.css">
  <link rel="stylesheet" type="text/css" href="css/timedropper.min.css">
  <script src="js/datedropper.min.js"></script>
  <script src="js/timedropper.min.js"></script>

  <!-- Network -->
  <li> Configure </li>
  <div style="margin-left:40px;">
    <table>
        <!--
      <tr>
        <td>
          <div onClick="javascript:dhcpRadioClick()" style="width:80px;">
            <input name="IPSettings" id="IPSettingsDHCP" type="radio" value="DHCP" 
            <?php
                $command=$MiniOS->configs["network"]["dhcp"]["check_mode"][$MiniOS->system_type];
                $dhcpGetIP = exec ($command);
                if ($dhcpGetIP != null) {
                    echo "checked";
                }
            ?>
            ><span id=aa onClick="IPSettingsDHCP.checked=true">DHCP</span>
          </div>
        </td>
        <td></td>
        <td rowspan="2">
          <input name="networkSubmit" type="button" onClick="javascript:setNetworkConfigure()" value="Submit">
        </td>
      </tr>
        -->
      <tr>
        <td>
          <div onClick="javascript:staticIPRadioClick()" style="width:80px;">
              <input name="IPSettings" id="IPSettingsStaticIP"type="radio" value="StaticIP"
              <?php
                  $command=$MiniOS->configs["network"]["static"]["check_mode"][$MiniOS->system_type];
                  $dhcpGetIP = exec ($command);
                  if ($dhcpGetIP != null) {
                      echo "checked";
                  }
              ?>
              ><span id=aa onClick="IPSettingsStaticIP.checked=true">Static IP</span>
          </div>
        </td>
        <td>
          <div id="staticSettingsAglinDiv">
            <table>
              <tr>
                <td>IP:</td>
                <td>
                  <input style="text-align:center;" name="ip" type="text" value=
                      <?php
                          $command=$MiniOS->configs["network"]["static"]["ip"][$MiniOS->system_type];
                          $localIP = exec ($command);
                          echo "\"".$localIP."\"";
                          ?>
                      >
                </td>
                <td>
                  <?php
                    $command = "grep ".$localIP." /etc/network/interfaces";
                    $check = exec ($command);

                    if ($check == null)
                        echo "<img src='img/error.png' width='30' height='30'/>";
                    else
                        echo "<img src='img/ok.png' width='30' height='30'/>";
                  ?>
                </td>
              </tr>
              <tr>
                <td>Netmask:</td>
                <td>
                <input style="text-align:center;" name="netmask" type="text" value=
                    <?php
                        $command=$MiniOS->configs["network"]["static"]["netmask"][$MiniOS->system_type];
                        $localIP = exec ($command);
                        echo "\"".$localIP."\"";
                        ?>
                    >
                </td>
                <td>
                  <?php
                    $command = "grep ".$localIP." /etc/network/interfaces";
                    $check = exec ($command);

                    if ($check == null)
                        echo "<img src='img/error.png' width='30' height='30'/>";
                    else
                        echo "<img src='img/ok.png' width='30' height='30'/>";
                  ?>
                </td>
              </tr>
              <tr>
                <td>Broadcast:</td>
                <td>
                <input style="text-align:center;" name="broadcast" type="text" value=
                    <?php
                        $command=$MiniOS->configs["network"]["static"]["broadcast"][$MiniOS->system_type];
                        $localIP = exec ($command);
                        echo "\"".$localIP."\"";
                        ?>
                    >
                </td>
                <td>
                  <?php
                    $command = "grep ".$localIP." /etc/network/interfaces";
                    $check = exec ($command);

                    if ($check == null)
                        echo "<img src='img/error.png' width='30' height='30'/>";
                    else
                        echo "<img src='img/ok.png' width='30' height='30'/>";
                  ?>
                </td>
              </tr>
              <tr>
                <td>Gateway:</td>
                <td>
                <input style="text-align:center;" name="gateway" type="text" value=
                    <?php
                        $command=$MiniOS->configs["network"]["static"]["gateway"][$MiniOS->system_type];
                        $localIP = exec ($command);
                        echo "\"".$localIP."\"";
                        ?>
                    >
                </td>
                <td>
                  <?php
                    $command = "grep ".$localIP." /etc/network/interfaces";
                    $check = exec ($command);

                    if ($check == null)
                        echo "<img src='img/error.png' width='30' height='30'/>";
                    else
                        echo "<img src='img/ok.png' width='30' height='30'/>";
                  ?>
                </td>
              </tr>
            </table>
          </div>
        </td>
      </tr>
    </table>
  </div>
  <!--
  <li>Check the WAN network</li> 
  <div style="margin-left:40px;">
    <table>
      <tr>
        <td>
          IP or DNS:
        </td>
        <td>
          <input style="text-align:center;" name="pingNetWork" type="text" value=
              <?php
                  echo $MiniOS->configs["network"]["ping"]["ip"];
              ?>
              >
        </td>
        <td>
          <input type="button" onClick="javascript:pingNetWork()" value="Ping">
          <?php
            echo "<img src='img/error.png' width='30' height='30'/>";
          ?>
        </td>
      </tr>
    </table>
  </div>
  -->
</ul>
<hr/>
