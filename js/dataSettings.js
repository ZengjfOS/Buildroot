function checkIP(ip) 
{ 
    obj=ip; 
    var exp=/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/; 
    var reg = obj.match(exp); 
    if(reg==null) { 
        return false;//不合法 
    } else { 
        return true; //合法 
    } 
} 

function checkMask(mask) 
{ 
    obj=mask; 
    var exp=/^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/; 
    var reg = obj.match(exp); 
    if(reg==null) { 
        return false; //"非法" 
    } else { 
        return true; //"合法" 
    } 
} 

function send_ajax_data(json_data, success_function)
{

    if (success_function == null) {
        console.info("Please pass send_ajax_data function as argument.");
        return ;
    }

    $.ajax({
        url: "settings.php",
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

function setNetworkConfigure()
{
    var value = $('input[type="radio"][name="IPSettings"]:checked').val();
    console.info(value);

    var ajaxPostData = {};
    if (value == "DHCP"){
        console.info("execute DHCP.");
        ajaxPostData = {"categories":"network", "type": "dhcp"};
        console.info(ajaxPostData);
    } else {
        ip = $('input[name="ip"]').val();
        netmask = $('input[name="netmask"]').val();
        broadcast = $('input[name="broadcast"]').val();
        gateway = $('input[name="gateway"]').val();
        ajaxPostData = {"categories":"network", "type": "staticIP", "ip":ip, "netmask":netmask, "broadcast":broadcast, "gateway":gateway}
        console.info(ajaxPostData);

        if (checkIP(ip) == false) {
            alert("Please Check Your IP Format.")
            return;
        }

        if (checkIP(netmask) == false) {
            alert("Please Check Your Network Format.")
            return;
        }

        if (checkIP(broadcast) == false) {
            alert("Please Check Your Broadcast Format.")
            return;
        }

        if (checkIP(gateway) == false) {
            alert("Please Check Your Gateway Format.")
            return;
        }
    }

    send_ajax_data(ajaxPostData, 
        function(data){
            alert("Settings is Ok. The Machine is rebooting.");
        },
    );
}

$("#pickdate").dateDropper({
	animate: false,
	format: 'Y-m-d',
	maxYear: '2020'
});
$("#picktime").timeDropper({
	meridians: false,
	format: 'HH:mm',
});

function setDataAndTime()
{
    date = $("#pickdate").val();
    time = $("#picktime").val();
    
    ajaxPostData = {"categories":"dateAndTime", "type": "dateAndTime", "date":date, "time":time};
    console.info(ajaxPostData);

    send_ajax_data(ajaxPostData, 
        function(data){
            alert("Data and Time Set is Ok.");
        }
    );
}

function dhcpRadioClick()
{
    $('input[type="radio"][name="IPSettings"]').filter('[value=DHCP]').prop('checked', true);
    $("#staticSettingsAglinDiv").hide();
    
}

function staticIPRadioClick()
{
    $('input[type="radio"][name="IPSettings"]').filter('[value=StaticIP]').prop('checked', true);
    $("#staticSettingsAglinDiv").show();
}

function pingNetWork()
{
    netmask = $('input[name="pingNetWork"]').val();

    ajaxPostData = {"categories":"network", "type": "ping", "IPOrDNS": "www.baidu.com"};

    send_ajax_data(ajaxPostData, 
        function(data){
            console.info("Ping to WAN is OK, data[status]:" + data["status"] + ".");
        }
    );
}

function enableCoverMask(){
    $('.bg').css({'display':'block'});
    $('.content').css({'display':'block'});
}

function disableCoverMask(){
    $('.bg').css({'display':'none'});
    $('.content').css({'display':'none'});
}

$(function(){  
    var value = $('input[type="radio"][name="IPSettings"]:checked').val();

    if (value == "DHCP"){
        $("#staticSettingsAglinDiv").hide();

        $('input[name="ip"]').val("");
        $('input[name="netmask"]').val("");
        $('input[name="broadcast"]').val("");
        $('input[name="gateway"]').val("");
    } else {
        $("#staticSettingsAglinDiv").show();
    }

    ajaxPostData = {"categories":"hardware_test", "type": "test"};

    send_ajax_data(ajaxPostData, 
        function(data){
            // output for check data
            console.info(data);

            // gpio
            if (data["data"]["gpio"]["status"] == "ok") {
                $("#" + data["data"]["gpio"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // eeprom
            if (data["data"]["eeprom"]["status"] == "ok") {
                $("#" + data["data"]["eeprom"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // tmp75
            if (data["data"]["tmp75"]["status"] == "ok") {
                console.info($("#" + data["data"]["tmp75"]["name"] + "_value")); 
                $("#" + data["data"]["tmp75"]["name"] + "_value").html(data["data"]["tmp75"]["value"]); 
                $("#" + data["data"]["tmp75"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // rtc
            if (data["data"]["rtc"]["status"] == "ok") {
                console.info($("#" + data["data"]["rtc"]["name"] + "_value")); 
                $("#" + data["data"]["rtc"]["name"] + "_value").html(data["data"]["rtc"]["value"]); 
                $("#" + data["data"]["rtc"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // DB9_RS232 us100
            if (data["data"]["rtc"]["status"] == "ok") {
                console.info($("#" + data["data"]["us100"]["name"] + "_value")); 
                $("#" + data["data"]["us100"]["name"] + "_value").html(data["data"]["us100"]["value"]); 
                $("#" + data["data"]["us100"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // keyboard
            if (data["data"]["input"]["keyboard"]["status"] == "ok") {
                console.info(data["data"]["input"]["keyboard"]["name"] + "_value"); 
                $("#" + data["data"]["input"]["keyboard"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // mouse
            if (data["data"]["input"]["mouse"]["status"] == "ok") {
                console.info(data["data"]["input"]["mouse"]["name"] + "_value"); 
                $("#" + data["data"]["input"]["mouse"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // usb SD1
            if (data["data"]["udisk"]["usb1"]["status"] == "ok") {
                console.info(data["data"]["udisk"]["usb1"]["name"] + "_value"); 
                $("#" + data["data"]["udisk"]["usb1"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // usb SD2
            if (data["data"]["udisk"]["usb2"]["status"] == "ok") {
                console.info(data["data"]["udisk"]["usb2"]["name"] + "_value"); 
                $("#" + data["data"]["udisk"]["usb2"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // network1
            if (data["data"]["network1"]["status"] == "ok") {
                console.info(data["data"]["network1"]["name"] + "_value"); 
                $("#" + data["data"]["network1"]["name"] + "_status").attr("src", "img/ok.png");
            }

            // network2
            if (data["data"]["network2"]["status"] == "ok") {
                console.info(data["data"]["network2"]["name"] + "_value"); 
                $("#" + data["data"]["network2"]["name"] + "_status").attr("src", "img/ok.png");
            }
        }
    );
}); 

