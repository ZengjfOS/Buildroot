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
			// console.info("ajax back infomations success.");
			// console.info(data);

			if (data["status"] == "ok"){
				success_function(data);
			} else {
				console.log("Please Go To Console Get More Infomations.");
				GPIOWhileLoop = false;

				console.info(data);
			}
		},
		error: function(xhr, ajaxOptions, thrownError) {
			//On error do this
			console.log("Please Go To Console Get More Infomations.");
		GPIOWhileLoop = false;

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

	sendData= {"categories":"hardware_gpio", "type": "output", "index": index, "value": val};
	ws.send(JSON.stringify(sendData));
}

color_array = ["green", "orange", "blue", "white"];

function timedCount()
{
	if ($('[name="output_0"]').length != 1) {
		console.log("left GPIO part.");
		return;
	};

	sendData = {"categories":"hardware_gpio", "type": "input"};
	ws.send(JSON.stringify(sendData));

	setTimeout("timedCount()",3000);
}

$(function(){
	console.log($("#sp_spacing_div"));
	$("#sp_spacing_div").on("remove", function() {
		alert("Element was removed");
	});
	timedCount();

	// auto to center
	if (document.documentElement.clientHeight > 738) {
		$("#sp_spacing_div").height((document.documentElement.clientHeight / 2) - (738 / 2));
	}

	ajaxPostData = {"categories":"hardware_gpio", "type": "input"};
	send_ajax_data(frame_argv["path"] + "/backend.php", ajaxPostData, 
		function(data){
			console.log(data);
			for (var i = 0; i < color_array.length; i++) {
				img = document.getElementsByName("output_" + i);
				var fileName = getFileName(img[0].src);
				var imgName = img[0].name;
				
				if (data["status"] == "ok") {
					if (data["gpios"]["in_" + i]["status"] == "ok" && data["gpios"]["in_" + i]["value"] == 0) {
						img[0].src = "img/power_blue.png";
					} else {
						img[0].src = "img/power_gray.png";
					}
				}
			}
		}
	);
});


// framework callback
function GPIO_init(argv) {
	frame_argv = argv;
	console.log(argv);

	aplexos.ws.onmessage = function(e) {
		ret = JSON.parse(e.data);
		if (ret["type"] == "output" ) {
			if (ret["status"] == "ok") {
				img = $('img[name="output_' + ret["index"] + '"]')[0];
				var fileName = getFileName(img.src);
				if (fileName == "power_gray.png")
					img.src = "img/power_blue.png";
				else
					img.src = "img/power_gray.png";
			}
		}

		if (ret["type"] == "input" ) {
			for (var i = 0; i < color_array.length; i++) {
				if (ret["gpios"] != undefined) {
					img = document.getElementsByName("module1_led_" + color_array[i]);
					var fileName = getFileName(img[0].src);
					var imgName = img[0].name;
				
					if (ret["gpios"]["in_" + i]["status"] == "ok" && ret["gpios"]["in_" + i]["value"] == "0") {
						img[0].src = "img/led_gray.png"
					} else {
						var moduleColor = imgName.substring(imgName.lastIndexOf("_") + 1);	
						img[0].src = "img/led_" + moduleColor + ".png";
					}
				}
			}
		}
	}
}
