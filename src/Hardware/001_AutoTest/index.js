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

$(function(){ 
    console.log("Auto Test. -- zengjf");
    $.getJSON(frame_argv["path"] + "/config.json", function(data) {
        MiniConfig = data;

        console.log(MiniConfig);

        htmlString = "";
        i = 1;
        _.each(MiniConfig["system_info"], (item, key, list) => {
            // console.log(list);
            htmlString += "<tr>";
            htmlString += "  <td class='systemInfoTable systemInfoTableIndex'>" + i++ + "</td>\n";
            htmlString += "  <td class='systemInfoTable systemInfoTableType'>" + key + ":</td>\n";
            htmlString += "  <td class='systemInfoTable systemInfoTableInfo' id='system_info_" + key + "'>Wating...</td>\n";
            htmlString += "  <td class='systemInfoTable systemInfoTableStatus'>";
            htmlString += "   <img src='img/error.png' width='30' height='30' id='" + key + "_status'/>";
            htmlString += "  </td>";
            htmlString += "</tr>";
        });
        $('#getSystemInfo').html(htmlString);
        htmlString = "";

        htmlString += "<tr>";
        htmlString += "  <th scope='col' class='hardwareInfo'>Index</th>";
        htmlString += "  <th scope='col' class='hardwareInfo'>Module</th>";
        htmlString += "  <th scope='col' class='hardwareInfo'>Descriptor</th>";
        htmlString += "  <th scope='col' class='hardwareInfo'>Status</th>";
        htmlString += "</tr>";
        _.each(MiniConfig["hardware_test"], (item, key, list) => {
              htmlString += "<tr>";
              htmlString += "<th scope='row' class='hardwareInfo hardwareInfoIndex'>" + item["index"] + "</th>";
              htmlString += "<td class='hardwareInfo hardwareInfoType'>" + key + "</td>";
              htmlString += "<td class='hardwareInfo hardwareInfoInfo'>" + item["descriptor"] + "</td>";
              htmlString += "<td class='hardwareInfo hardwareInfoStatus'>";
              htmlString += "<img src='img/" + item["status"] + ".png' width='30' height='30' id='" + key + "_status'/>";
              htmlString += "</td>";
              htmlString += "</tr>";
        });
        $('#getHardwareInfo').html(htmlString);
        htmlString = "";

        ajaxPostData = {"categories":"hardware_test", "type": "test"};
        send_ajax_data(frame_argv["path"] + "/backend.php", ajaxPostData, 
            function(data){

                var test_items = Object.keys(data["data"]["hardware_test"]);
		console.log(test_items);
                for (i = 0; i < test_items.length; i++) {
                    item = test_items[i];
                    if (data["data"]["hardware_test"].hasOwnProperty(item)) {
                        if (data["data"]["hardware_test"][item]["status"] == "ok") {
                            $("#" + item + "_status").attr("src", "img/ok.png")
                        }
                    }
                }

                var test_items = Object.keys(data["data"]["system_info"]);
		console.log(test_items);
                for (i = 0; i < test_items.length; i++) {
                    item = test_items[i];
                    if (data["data"]["system_info"].hasOwnProperty(item)) {
                        if (data["data"]["system_info"][item]["status"] == "ok") {
                            $("#system_info_" + item)[0].innerHTML = data["data"]["system_info"][item]["result"];
                            $("#" + item + "_status").attr("src", "img/ok.png")
                        }
                    }
                }
            }
        );
    });
});

function AutoTest_init(argv) {
    frame_argv = argv;
    console.log(argv);
}
