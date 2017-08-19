<h2 id="hardware-test">Hardware Test</h2>
<div>
  <style>
    td {
      text-align: center;
    }
  </style>
  <table width="100%" border="1" align="center" >
    <tr>
      <th scope="col">Index</th>
      <th scope="col">Module</th>
      <th scope="col">Port</th>
      <th scope="col">Range</th>
      <th scope="col">Value</th>
      <th scope="col">Status</th>
    </tr>
    <?php
      foreach ($MiniOS->configs["hardware_test"] as $key => $value) {
          echo "<tr>";
          echo "<th scope='row'>".$value["index"]."</th>";
          echo "<td>".$key."</td>";
          echo "<td>".$value["descriptor"]."</td>";
          echo "<td>".$value["range"]."</td>";
          echo "<td id='".$key."_value'>".$value["value"]."</td>";
          echo "<td>";
          echo "<img src='img/".$value["status"].".png' width='30' height='30' id='".$key."_status'/>";
          echo "</td>";
          echo "</tr>";
      }
    ?>
  </table>
<div>
