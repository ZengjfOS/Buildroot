var current_language ="";

function replaceAll(str, find, replace) {
    return str.replace(new RegExp(find, 'g'), replace);
}

function show_nav_frame() {
    $.get('templates/nav.html', function(src) {
        var template = _.template(src);
        $('#bs-example-navbar-collapse-1').html(template({"nav":configs["nav"]}));
    });
}

function IsURL(str_url){
    var strRegex = /[-a-zA-Z0-9@:%_\+.~#?&//=]{2,256}\.[a-z]{2,4}\b(\/[-a-zA-Z0-9@:%_\+.~#?&//=]*)?/gi;

    var re=new RegExp(strRegex); 
    if (re.test(str_url))
        return (true); 
    else
        return (false); 
}

function getNavigationMapsValue(map_key) {
    var value = "";
    if (current_language.indexOf("en") != -1) {
        return map_key;
    } else {
        _.each(configs.nav.maps, (item, key, list) => {
            if (_.keys(item)[0] == map_key) {
                value = (item[map_key]);
            }
        });
    }

    return value;
}

function nav_click_search_content(obj){ 
    frame_type = getNavigationMapsValue(obj.parentNode.parentNode.parentNode.getElementsByTagName("a")[0].text);
    demo_name  = obj.innerHTML;
    current_page = configs["nav"][frame_type]["pages"][obj.innerHTML];
    path_name  = current_page["index"] + "_" + obj.innerHTML;

    demo_css  = 'src/' + frame_type + '/' + path_name + '/index.css';
    demo_js   = 'src/' + frame_type + '/' + path_name + '/index.js';
    demo_html = 'src/' + frame_type + '/' + path_name + '/index.html';
    demo_path = 'src/' + frame_type + '/' + path_name;

    if (current_page.hasOwnProperty('frameType')) {
        template_css  = 'templates/' + current_page['frameType'] + '/' + current_page['frameType'] + '.css';
        template_html  = 'templates/' + current_page['frameType'] + '/' + current_page['frameType'] + '.html';
    } else {
        template_css  = 'templates/common/common.css';
        template_html = 'templates/common/common.html';
    }

    dynamic_get_CSS(demo_css);
    dynamic_get_CSS(template_css);

    $.get(template_html, function(src) {
        var template = _.template(src);
        $('#show-content').html(template({"title" : demo_name}));

        $.get(demo_html, function(result) {
            // show html
            $('#show-content_render').html(result);

            // finish html and get javascript to execute
            dynamic_get_script(demo_name, demo_js, {"type": frame_type, "name": demo_name, "path": demo_path});
        }); 
    });
}

function show_home_page(obj){ 

    if (configs.hasOwnProperty('home_page')
            && configs['home_page'].hasOwnProperty('show')
            && configs['home_page']['show']) {

        _.each(configs['home_page'], (val_e, key_e) => {
            current_page = configs['home_page'];
            frame_type = key_e;
            _.each(val_e["pages"], (val_i, key_i) => {
                demo_name = key_i;
                path_name = val_i["index"] + "_" + demo_name;

                demo_css  = 'src/' + frame_type + '/' + path_name + '/index.css';
                demo_js   = 'src/' + frame_type + '/' + path_name + '/index.js';
                demo_html = 'src/' + frame_type + '/' + path_name + '/index.html';
                demo_path = 'src/' + frame_type + '/' + path_name;

                if (current_page.hasOwnProperty('frameType')) {
                    template_css  = 'templates/' + current_page['frameType'] + '/' + current_page['frameType'] + '.css';
                    template_html  = 'templates/' + current_page['frameType'] + '/' + current_page['frameType'] + '.html';
                } else {
                    template_css  = 'templates/common/common.css';
                    template_html = 'templates/common/common.html';
                }

                dynamic_get_CSS(demo_css);
                dynamic_get_CSS(template_css);

                $.get(template_html, function(src) {
                    var template = _.template(src);
                    $('#show-content').html(template({"title" : demo_name}));

                    $.get(demo_html, function(result) {
                        // show html
                        $('#show-content_render').html(result);

                        // finish html and get javascript to execute
                        dynamic_get_script(demo_name, demo_js, {"type": frame_type, "name": demo_name, "path": demo_path});
                    }); 
                });
            });
        });

        // just show a time
        configs['home_page']['show'] = false;
    } else {
        $.ajax({ 
            async:false, 
            url : "templates/home_page.html", 
            success : function(src){ 
                // show home_page.html
                var template = _.template(src);
                $('#show-content').html(template({"title" : configs["title"]}));
            } 
        }); 
    }
} 

function footer_position(){
    $("footer").removeClass("fixed-bottom");

    var contentHeight = document.body.scrollHeight, //网页正文全文高度
        winHeight = window.innerHeight;             //可视窗口高度，不包括浏览器顶部工具栏
    if(!(contentHeight > winHeight)){
        //当网页正文高度小于可视窗口高度时，为footer添加类fixed-bottom
        $("footer").addClass("fixed-bottom");
    }
}

function call_string_function(function_name, json_data) {
    // string as a function call
    var fn = window[function_name]; 
    if(typeof fn === 'function') 
        fn(json_data);
}

// dynamic get javascript and run the demo_name function in script file.
jQuery.loadScript = function (url, callback) {
    jQuery.ajax({
        url: url,
        dataType: 'script',
        success: callback,
        async: true
    });
}
function dynamic_get_script(demo_name, demo_js, json_data) {

    if (typeof someObject == 'undefined') $.loadScript(demo_js, function(result){
        //Stuff to do after someScript has loaded
        // console.log(demo_name);
        // console.log(json_data);
        call_string_function(demo_name + '_init', json_data);
    });
}

function dynamic_get_CSS (file_path) {
    $('head').append('<link rel="stylesheet" type="text/css" href="' + file_path + '">');
}

function nav_language_change(obj) {
    var language = "";
    if (obj == undefined ) {
        language = $.i18n.normaliseLanguageCode({"language" : ""});
    } else {
        language = obj.innerHTML.toString();
        $.i18n.normaliseLanguageCode({"language": language});
    }

    current_language = language;

    // This will initialize the plugin 
    // and show two dialog boxes: one with the text "Olá World"
    // and other with the text "Good morning John!" 
    // How to dynamically change language using jquery-i18n-properties and JavaScript?
    //    https://stackoverflow.com/questions/15637059/how-to-dynamically-change-language-using-jquery-i18n-properties-and-javascript
    jQuery.i18n.properties({
        name:'lang', 
        path:'language/', 
        mode:'both',
        language: language,
        async: true,
        callback: function() {
            // We specified mode: 'both' so translated values will be
            // available as JS vars/functions and as a map

            _.each(configs.nav.parts, (item, key, list) => {
                $('.lang_' + item)[0].childNodes[0].nodeValue = jQuery.i18n.prop('lang_' + item);
            });

            /*
            _.each(configs.nav.Hardware.pages, (val_e, key_e) => {
                $('.lang_' + item)[0].childNodes[0].nodeValue = jQuery.i18n.prop('lang_' + item);
                console.log(val_e);
                console.log(key_e);
            });
            */

            $(".lang_language")[0].childNodes[0].nodeValue = jQuery.i18n.prop("lang_language");
        }
    });
}

class AplexOS {
    constructor() {
        console.log("AplexOS")
        this.config = null;
        this.ws = null;
    }
}

aplexos = new AplexOS();

function wss_init() {
    ws = new WebSocket("ws://" + configs["websocket"]["wss_ip"] + ":" + configs["websocket"]["wss_port"] + "/");
    ws.onopen = function() {
        console.log("onopen");
    }

    ws.onmessage = function(e) {
        console.log("onmessage: " + e.data);
    }

    ws.onclose = function() {
        console.log("onclose");
    }

    ws.onerror = function() {
        console.log("onerror");
    }

    aplexos.ws = ws;
}

$(function(){ 
    show_nav_frame();

    // demo string as a function call
    // show_home_page();
    call_string_function("show_home_page");

    footer_position();
    $(window).resize(footer_position);

    marked.setOptions({
        renderer: new marked.Renderer(),
        gfm: true,
        tables: true,
        breaks: false,
        pedantic: false,
        sanitize: false,
        smartLists: true,
        smartypants: false
    });

    nav_language_change();

    aplexos.config = configs;
    wss_init();
});

