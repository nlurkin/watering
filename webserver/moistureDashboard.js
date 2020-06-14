/**
 * 
 */

function update() {
	update_console();
}

function update_console() {
	const Url="http://localhost:8000/";
	$.ajax({
		url: Url+"console",
		type: "GET",
		success: function(result){ result = result.replace(/\r/g, "").replace(/\n/g, "\n"); document.getElementById("arduino_text").innerHTML = result; },
		error: function(error){ console.log(`Error ${error}`) }
		   })
	$.ajax({
		url: Url+"MQTT",
		type: "GET",
		success: function(result){ result = result.replace(/\r/g, "").replace(/\n/g, "\n"); document.getElementById("arduino_mqtt").innerHTML = result; },
		error: function(error){ console.log(`Error ${error}`) }
		   })
}

function clear_buffer() {
	const Url="http://localhost:8000";
	$.ajax({
		url: Url,
		type: "DELETE",
		crossDomain : true,
		success: function(result){ console.log(result); update(); },
		error: function(error){ console.log(`Error ${error}`) }
		   })
}

function submit_cmd() {
	const Url="http://localhost:8000";
	$.ajax({
		url: Url,
		type: "PUT",
		data: document.getElementById("arduino_cmd").value,
		success: function(result){ console.log(result) },
		error: function(error){ console.log(`Error ${error}`) }
		   })
}
