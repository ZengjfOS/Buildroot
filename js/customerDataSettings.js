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

function enableCoverMask(){
    $('.bg').css({'display':'block'});
    $('.content').css({'display':'block'});
}

function disableCoverMask(){
    $('.bg').css({'display':'none'});
    $('.content').css({'display':'none'});
}

function customerDate() 
{ 
    remoteIP = $('input[name="remoteIP"]').val();

    ajaxPostData = {"categories":"customer", "type": "configure", "remoteIP": remoteIP};

    console.info(ajaxPostData);

    $.ajax({
        url: "customerDataSettings.php",
        type: 'POST',
        contentType:'application/json; charset=utf-8',
        data: JSON.stringify(ajaxPostData),
        dataType:'json',
        success: function(data){
            //On ajax success do this
            console.info("success.");
            console.info(data);
            console.info("zengjf");
            if (data["status"] == "ok"){
                alert("Set remote IP is OK.");
            }
        },
        error: function(xhr, ajaxOptions, thrownError) {
            //On error do this
            console.info("error.");
            if (xhr.status == 200) {
    
                alert(ajaxOptions);
            }
            else {
                alert(xhr.status);
                alert(thrownError);
            }
        }
    });
}
function customer_config_update(){
    RemoteIP = $('input[name="RemoteIP"]').val();
    RemotePort = $('input[name="RemotePort"]').val();
    RemoteUser = $('input[name="RemoteUser"]').val();
    RemotePassword = $('input[name="RemotePassword"]').val();
    RemoteDatabase = $('input[name="RemoteDatabase"]').val();
    RemoteTable = $('input[name="RemoteTable"]').val();
    LocalIP = $('input[name="LocalIP"]').val();
    LocalPort = $('input[name="LocalPort"]').val();
    LocalUser = $('input[name="LocalUser"]').val();
    LocalPassword = $('input[name="LocalPassword"]').val();
    LocalDatabase = $('input[name="LocalDatabase"]').val();
    LocalTable = $('input[name="LocalTable"]').val();
    DBHeartbeat = $('input[name="DBHeartbeat"]').val();
    DBUpdate = $('input[name="DBUpdate"]').val();

    /*
    console.info("RemoteIP: " + RemoteIP + "\n" +
            "RemotePort: " + RemotePort + "\n" +
            "RemoteUser: " + RemoteUser + "\n" +
            "RemotePassword: " + RemotePassword + "\n" +
            "RemoteDatabase: " + RemoteDatabase + "\n" +
            "RemoteTable: " + RemoteTable + "\n" +
            "LocalIP: " + LocalIP + "\n" +
            "LocalPort: " + LocalPort + "\n" +
            "LocalUser: " + LocalUser + "\n" +
            "LocalPassword: " + LocalPassword + "\n" +
            "LocalDatabase: " + LocalDatabase + "\n" +
            "LocalTable: " + LocalTable + "\n" +
            "DBHeartbeat: " + DBHeartbeat + "\n" +
            "DBUpdate: " + DBUpdate);
            */

    if(!checkIP(RemoteIP)){
        alert("Please Check Your Remote IP Address Format.")
        return 
    }

    if(!checkIP(LocalIP)){
        alert("Please Check Your Local IP Address Format.")
        return 
    }

    ajaxPostData = {"categories":"customer", 
        "type": "configure", 
        "remote":{
            "ip":RemoteIP,
            "port":RemotePort,
            "user":RemoteUser,
            "password":RemotePassword,
            "database":RemoteDatabase,
            "table":RemoteTable,
        },
        "localhost":{
            "ip":LocalIP,
            "port":LocalPort,
            "user":LocalUser,
            "password":LocalPassword,
            "database":LocalDatabase,
            "table":LocalTable,
        }, 
        "interval":{
            "heartbeat":DBHeartbeat,
            "upload_data":DBUpdate,
        },
    };

    console.info(JSON.stringify(ajaxPostData));

    $.ajax({
        url: "customerDataSettings.php",
        type: 'POST',
        contentType:'application/json; charset=utf-8',
        data: JSON.stringify(ajaxPostData),
        dataType:'json',
        success: function(data){
            //On ajax success do this
            disableCoverMask();
            if (data["status"] == "ok"){
                alert("Update configure is OK.");
            } else {
                alert("Update configure is Faile.");
            }
        },
        error: function(xhr, ajaxOptions, thrownError) {
            //On error do this
            console.info("error.");
            disableCoverMask();
            if (xhr.status == 200) {
    
                alert(ajaxOptions);
            }
            else {
                alert(xhr.status);
                alert(thrownError);
            }
        }
    });
}

function updateApp(){

    ftpIPAddress = $('input[name="ftpIPAddress"]').val();
    ftpApplicationName = $('input[name="ftpApplicationName"]').val();

    enableCoverMask();

    ajaxPostData = {"categories":"customer", "type": "updateApp", "ftpIPAddress": ftpIPAddress, "appName": ftpApplicationName};

    console.info(ajaxPostData);

    $.ajax({
        url: "customerDataSettings.php",
        type: 'POST',
        contentType:'application/json; charset=utf-8',
        data: JSON.stringify(ajaxPostData),
        dataType:'json',
        success: function(data){
            //On ajax success do this
            disableCoverMask();
            if (data["status"] == "ok"){
                alert("Update Application is OK.");
            } else {
                alert("Update Application is Faile.");
            }
        },
        error: function(xhr, ajaxOptions, thrownError) {
            //On error do this
            console.info("error.");
            disableCoverMask();
            if (xhr.status == 200) {
    
                alert(ajaxOptions);
            }
            else {
                alert(xhr.status);
                alert(thrownError);
            }
        }
    });
}
