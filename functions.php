<?php

// author: zengjf
// date  : Tue Aug  1 12:28:41 CST 2017

class MiniOS_context {

    // file path for json configure
    public $config_file_path = "configs/system_config.json";

    // json data format
    public $configs = null;

    // 获取系统类型，X86，ARM
    public $system_type = "Linux";

    public function __construct() {

        // parse configure to json data
        $this->configs = json_decode(file_get_contents($this->config_file_path), true);

        // get running system type
        $this->system_type = exec ("uname -m");
    }

    public function get_configs() {
        if($this->configs==null)
            $this->configs = json_decode(file_get_contents($this->config_file_path), true);

        return $this->configs;
    }

    // how to use:
    //    print_r($MiniOS->get_config_section_value("SystemInfo"));
    public function get_config_section_value($key, $configs = null) {
        if ($configs == null) {
            return $this->configs[$key];
        } else {
            return $configs[$key];
        }
    }

    // get all config sections
    public function get_config_sections($key, $configs = null) {
        $sections = array();

        if ($configs == null) {
            foreach ($this->configs as $section => $section_value){
                $sections[] = $section;
            }
        } else {
            foreach ($configs as $section => $section_value){
                $sections[] = $section;
            }
        }

        print_r($sections);
    }
}

$MiniOS = new MiniOS_context;

?> 
