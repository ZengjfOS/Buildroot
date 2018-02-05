function getFileName(filePath){  
    var pos = filePath.lastIndexOf("/");  
    return filePath.substring(pos+1);    
}  

function set_DO_Value(img) {

    // console.log(img.src);
    var fileName = getFileName(img.src);
    if (fileName == "power_gray.png") {
        img.src = "img/power_blue.png";
    } else {
        img.src = "img/power_gray.png";
    }
}

function randomPowerStatus(img) {

    var fileName = getFileName(img[0].src);
    var imgName = img[0].name;
    
    if (((Math.random() * (10 - 1) + 1) / 5) > 1) {
        if (fileName == "led_green.png" || fileName == "led_orange.png" || fileName == "led_blue.png" || fileName == "led_white.png") {
            img[0].src = "img/led_gray.png"
        } else {
            var moduleColor = imgName.substring(imgName.lastIndexOf("_") + 1);    
            img[0].src = "img/led_" + moduleColor + ".png";
        }
    }
}

color_array = ["green", "orange", "blue", "white"];

function timedCount()
{

    for (var i = 0; i < color_array.length; i++) {
        randomPowerStatus(document.getElementsByName("module1_led_" + color_array[i]));
    }

    setTimeout("timedCount()",1000);
}

$(function(){
    function footerPosition(){
        $("footer").removeClass("fixed-bottom");
        var contentHeight = document.body.scrollHeight,//网页正文全文高度
            winHeight = window.innerHeight;//可视窗口高度，不包括浏览器顶部工具栏
        if(!(contentHeight > winHeight)){
            //当网页正文高度小于可视窗口高度时，为footer添加类fixed-bottom
            $("footer").addClass("fixed-bottom");
        }
    }

    footerPosition();
    $(window).resize(footerPosition);
    timedCount();

    // auto to center
    if (document.documentElement.clientHeight > 738) {
        $("#sp_spacing_div").height((document.documentElement.clientHeight / 2) - (738 / 2));
    }
});


// framework callback
function GPIO_init(json_data) {
    console.info("AplexOS_IoT_Demo_Temperature: " + json_data["type"]);

}
