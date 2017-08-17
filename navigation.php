<nav data-toggle="wy-nav-shift" class="wy-nav-side stickynav">
  <div class="wy-side-nav-search">
    <a href="." class="icon icon-home"> ARM-Settings</a>
    <div role="search">
      <form id ="rtd-search-form" class="wy-form" action="./search.html" method="get">
        <input type="text" name="q" placeholder="Search docs" />
      </form>
    </div>
  </div>
  <div class="wy-menu wy-menu-vertical" data-spy="affix" role="navigation" aria-label="main navigation">
    <ul class="current">
      <li>
      <li class="toctree-l1 current">
        <a class="current" href=".">Home</a>
        <ul>
          <?php
                foreach ($MiniOS->configs["navigation"] as $key => $value) {
                    if ($key == "welcome") {
                        echo "<li class='toctree-l3'><a href='".$value["link"]."'>".$value["content"]."</a></li>\n";
                    } else {
                        echo "<li><a class='toctree-l4' href='".$value["link"]."'>".$value["content"]."</a></li>\n";
                    }

                }
          ?>
        </ul>
      </li>
      <li>
    </ul>
  </div>
  &nbsp;
</nav>
