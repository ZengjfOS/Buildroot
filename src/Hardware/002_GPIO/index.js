$(function(){ 

    var div = document.getElementById('showContent');

    // Create a client instance
    var client = new Paho.MQTT.Client("zengjf.mqtt.iot.gz.baidubce.com", 8884, "DeviceId-" + Math.random().toString(36).substring(7));
    var server_connected = false;
    
    // set callback handlers
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    
    // connect the client
    client.connect({onSuccess:onConnect, onFailure:onConnectError, userName:"zengjf/sz_monitor_room", password:"QE0BHFvFnIkBRIaJtPYzo3m/63Esv5fzzMr9tYVOsHo=", useSSL:true});
    
    
    // called when the client connects
    function onConnect() {
      // Once a connection has been made, make a subscription and send a message.
      showMessageContent("onConnect:", "connect");
      client.subscribe("test-iot-sub");
      showMessageContent("subscribe:", "test-iot-sub");

      server_connected = true;
    }

    // called when the client connects
    function onConnectError() {
      // Once a connection has been made, make a subscription and send a message.
      showMessageContent("onConnectError:", "Error");
    }
    
    // called when the client loses its connection
    function onConnectionLost(responseObject) {
      if (responseObject.errorCode !== 0) {
        showMessageContent("onConnectionLost:", responseObject.errorMessage);

        // Create a client instance
        client = new Paho.MQTT.Client("zengjf.mqtt.iot.gz.baidubce.com", 8884, "DeviceId-" + Math.random().toString(36).substring(7));
        server_connected = false;
        
        // set callback handlers
        client.onConnectionLost = onConnectionLost;
        client.onMessageArrived = onMessageArrived;
        
        // connect the client
        client.connect({onSuccess:onConnect, onFailure:onConnectError, userName:"zengjf/sz_monitor_room", password:"QE0BHFvFnIkBRIaJtPYzo3m/63Esv5fzzMr9tYVOsHo=", useSSL:true});
      }
    }
    
    // called when a message arrives
    function onMessageArrived(message) {
      showMessageContent("onMessageArrived:", message.payloadString);

      stminfo = JSON.parse(message.payloadString);
      temperature.refresh(stminfo["temperature"]);
    }

    function showMessageContent(type, message) {
      div.innerHTML += "<div style='background : #999;'>" + type + "</div>"
      div.innerHTML += "<div>" + message + "</div>";
    }

    showMessageContent("Content init Ready:", "MQTT Client Set Over, Wait Data Tranfer.");

    var temperature = new JustGage({
      id: "gauge",
      value: 25,
      min: 0,
      max: 100,
      symbol: '℃',
    });
});

// framework callback
function GPIO_init(json_data) {
    console.info("AplexOS_IoT_Demo_Temperature: " + json_data["type"]);

}
