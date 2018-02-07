function fileSelected() {
    var file = document.getElementById('fileToUpload').files[0];

    if (file) {
        var fileSize = 0;

        if (file.size > 1024 * 1024)
            fileSize = (Math.round(file.size * 100 / (1024 * 1024)) / 100).toString() + 'MB';
        else
            fileSize = (Math.round(file.size * 100 / 1024) / 100).toString() + 'KB';
    }
}

function uploadFile(btn) {
    var fd = new FormData();
    type = btn.name.split("_")[1];
    json_data = {"categories":"upload", "type": type};

    fd.append("data", JSON.stringify(json_data));
    fd.append("fileToUpload", document.getElementById('fileToUpload').files[0]);

    var xhr = new XMLHttpRequest();

    xhr.upload.addEventListener("progress", uploadProgress, false);
    xhr.addEventListener("load", uploadComplete, false);
    xhr.addEventListener("error", uploadFailed, false);
    xhr.addEventListener("abort", uploadCanceled, false);

    console.log(frame_argv["path"]);
    xhr.open("POST", frame_argv["path"] + "/backend.php");

    xhr.send(fd);
}

function uploadProgress(evt) {
    if (evt.lengthComputable) {
        var percentComplete = Math.round(evt.loaded * 100 / evt.total);
        document.getElementById('progressBar').value = percentComplete;
        document.getElementById("progressBarPercent").innerHTML = percentComplete.toString() + "%";
    } else {
        document.getElementById('progressBar').value = 0;
        document.getElementById("progressBarPercent").innerHTML = "Unable to upload.";
    }
}

function uploadComplete(evt) {
    /* This event is raised when the server send back a response */
    document.getElementById("showBack").innerHTML = evt.target.responseText;
}

function uploadFailed(evt) {
    alert("There was an error attempting to upload the file.");
}

function uploadCanceled(evt) {
    alert("The upload has been canceled by the user or the browser dropped the connection.");
}

$(function(){ 

});

// framework callback
function UpdateSystem_init(argv) {
    frame_argv = argv;
    console.log(argv);
}
