function send_ajax_data(path, json_data, success_function)
{
    if (success_function == null) {
        console.info("Please pass send_ajax_data function as argument.");
        return ;
    }

    $.ajax({
        url: path,
        type: 'POST',
        contentType:'application/json; charset=utf-8',
        data: JSON.stringify(json_data),
        dataType:'json',
        success: function(data){
            //On ajax success do this
            console.info("ajax back infomations success.");
            console.info(data);

            if (data["status"] == "ok"){
                success_function(data);
            } else {
                alert("Please Go To Console Get More Infomations.");

                console.info(data);
            }
        },
        error: function(xhr, ajaxOptions, thrownError) {
            //On error do this
            alert("Please Go To Console Get More Infomations.");

            console.info(xhr);
            console.info(ajaxOptions);
            console.info(thrownError);
        }
    });
}

function getFileName(filePath){  
    var pos = filePath.lastIndexOf("/");  
    return filePath.substring(pos+1);    
}  

function set_DO_Value(img) {

    console.log(img.name);
    var index = img.name.split("_")[1];
    console.log(index);

    var fileName = getFileName(img.src);
    var val = 0;
    if (fileName != "power_gray.png") 
        val = 1;

    ajaxPostData = {"categories":"hardware_gpio", "type": "output", "index": index, "value": val};
    send_ajax_data(frame_argv["path"] + "/backend.php", ajaxPostData, 
        function(data){
            console.log(data);

            if (data["status"] == "ok") {
                var fileName = getFileName(img.src);
                if (fileName == "power_gray.png") {
                    img.src = "img/power_blue.png";
                } else {
                    img.src = "img/power_gray.png";
                }
            }

        }
    );

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

    /*
    for (var i = 0; i < color_array.length; i++) {
        randomPowerStatus(document.getElementsByName("module1_led_" + color_array[i]));
    }
    */

    ajaxPostData = {"categories":"hardware_gpio", "type": "input"};
    send_ajax_data(frame_argv["path"] + "/backend.php", ajaxPostData, 
        function(data){
            console.log(data);
        }
    );

    setTimeout("timedCount()",1000);
}

$(function(){
    timedCount();

    // auto to center
    if (document.documentElement.clientHeight > 738) {
        $("#sp_spacing_div").height((document.documentElement.clientHeight / 2) - (738 / 2));
    }
});


// framework callback
function GPIO_init(argv) {
    frame_argv = argv;
    console.log(argv);
}
