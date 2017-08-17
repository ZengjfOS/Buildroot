<h2 id="dateAndTime">Date & Time</h2>
<div>
  <table>
    <tr>
      <td>Date:</td>
      <td>
        <input type="text" class="input" id="pickdate" style="text-align: center;color: #000;border-color: #ccc;cursor: pointer;" value=
            <?php
                $command=$MiniOS->configs["date_and_time"]["date"][$MiniOS->system_type];
                $date = exec ($command);
                echo "\"".$date."\"";
                ?>
        />
        <br>
        <div align="center" style="clear: both;"></div>
      </td>
      <!--
      <td>
        <?php
          echo "<img src='img/ERROR.png' width='30' height='30'/>";
        ?>
      </td>
      -->
      <td rowspan="2">
        <input name="dataAndTimeSubmit" type="button" onClick="javascript:setDataAndTime()" value="Apply">
      </td>
    </tr>
    <tr>
      <td>Time:</td>
      <td>
        <input type="text" class="input" id="picktime" style="text-align:center;color:#000;border-color: #ccc;cursor: pointer;" value=
            <?php
                $command=$MiniOS->configs["date_and_time"]["time"][$MiniOS->system_type];
                $time = exec ($command);
                echo "\"".$time."\"";
                ?>
        />
      </td>
      <!--
      <td>
        <?php
          echo "<img src='img/ERROR.png' width='30' height='30'/>";
        ?>
      </td>
      -->
    </tr>
  </table>
</div>
<hr/>
