<body class="wy-body-for-nav" role="document">
  <style>
    .bg{display:none;position:fixed;width:100%;height:100%;background:#000;z-index:2;top:0;left:0;opacity:0.7;}
    .content{display:none;width:400px;height:100px;position:fixed;top:50%;margin-top:-150px;color:#F00;background:#000;z-index:3;left:50%;margin-left:-250px;padding:20px}
  </style>
  <div class="bg"></div>
  <div class="content">
    <li>Updating Is Runing.</li>
    <li>Don't Power Outages And Please Waiting is Over.</li>
    <li>Otherwise, The device is likely to be damaged.</li>
  </div>

  <div class="wy-grid-for-nav">
  <?php include 'navigation.php'; ?>
  <section data-toggle="wy-nav-shift" class="wy-nav-content-wrap">
    <nav class="wy-nav-top" role="navigation" aria-label="top navigation">
      <i data-toggle="wy-nav-top" class="fa fa-bars"></i>
      <a href=".">ARM-Settings</a>
    </nav>
    <div class="wy-nav-content">
    <div class="rst-content">
      <div role="navigation" aria-label="breadcrumbs navigation">
        <ul class="wy-breadcrumbs">
          <li><a href=".">Docs</a> &raquo;</li>
          <li>Home</li>
          <li class="wy-breadcrumbs-aside">
          </li>
        </ul>
        <hr/>
      </div>
      <div role="main">
        <div class="section">
          <h1 id="welcome-to-arm-settings">Welcome to ARM-Settings</h1>
          <p>This Page just for ARM machine Hardware Test.</p>
          <hr/>

          <?php include 'system_info.php'; ?>

          <?php include 'network.php'; ?>

          <?php include 'date_and_time.php'; ?>

          <!-- <?php include 'upload_system.php'; ?> -->

          <?php include 'hardware_test.php'; ?>

          <footer>
            <hr/>
            <div role="contentinfo">
                <!-- Copyright etc -->
            </div>
            Built with <a href="http://www.mkdocs.org">MkDocs</a> using a <a href="https://github.com/snide/sphinx_rtd_theme">theme</a> provided by <a href="https://readthedocs.org">Read the Docs</a>.
          </footer>
        </div>
      </div>
    </div>
  </section>
  </div>
  <div class="rst-versions" role="note" style="cursor: pointer">
    <span class="rst-current-version" data-toggle="rst-current-version">
    </span>
  </div>
  <script src="./js/theme.js"></script>

  <!-- for Settings -->
  <script type="text/javascript" src="js/dataSettings.js"></script>
</body>
