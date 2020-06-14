/**
 * 
 */

window.chartColors = {
	red: 'rgb(255, 99, 132)',
	orange: 'rgb(255, 159, 64)',
	yellow: 'rgb(255, 205, 86)',
	green: 'rgb(75, 192, 192)',
	blue: 'rgb(54, 162, 235)',
	purple: 'rgb(153, 102, 255)',
	grey: 'rgb(201, 203, 207)'
};

var charts_list = {};

function update() {
	update_console();
	Object.keys(charts_list).forEach(
			function (key,index) { update_chart(key, charts_list[key]); }
			);
}

function update_console() {
	const Url="http://localhost:8000/";
	$.ajax({
		url: Url+"console",
		type: "GET",
		success: function(result){ result = result.replace(/\r/g, "").replace(/\n/g, "\n"); document.getElementById("arduino_text").innerHTML = result; },
		error: function(error){ console.log(`Error ${error}`) }
		   })
//	$.ajax({
//		url: Url+"MQTT",
//		type: "GET",
//		success: function(result){ result = result.replace(/\r/g, "").replace(/\n/g, "\n"); document.getElementById("arduino_mqtt").innerHTML = result; },
//		error: function(error){ console.log(`Error ${error}`) }
//		   })
}

function update_chart(chart_name, chart) {
	const Url="http://localhost:8000/";
	labels = chart.data.labels
	last_time = labels[labels.length - 1]
	$.ajax({
		url: Url+"MQTT",
		type: "GET",
		data: { 
		    element: chart_name,
		    from: last_time
		},
		success: function(result){
			var obj = JSON.parse(result);
			addData(chart, obj["x"], obj["y"]);
			},
		error: function(error){ console.log(`Error ${error}`) }
		}
	)
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

function create_chart(name, display_name){
	var ctx = document.getElementById(name).getContext("2d");
	var config = {
			type: 'line',
			data: {
				labels: [],
				datasets: [{
					backgroundColor: window.chartColors.blue,
					borderColor: window.chartColors.blue,
					steppedLine: "middle",
					data: []
				}]
			},
			options: {
				legend: {display: false},
				responsive: true,
				title: {
					display: true,
					text: display_name
				},
				scales: {
					xAxes: [{
						type: 'time',
						autoSkip:true,
						time: {
							parser: 'YYYY-MM-DD HH:mm:SS',
							displayFormats: {
								minute: "HH:mm",
							},
							unit: "minute"
							},
					}],
				}
			}
		};
	charts_list[name] = new Chart(ctx,config);
}

function factorData(data, label) {
	function get_val(elmt) {
		if(elmt == undefined)
			return elmt;
		else if(elmt.constructor == Object)
			return elmt["y"];
		else
			return elmt;
	}
	var first_index = 0;
	let _data = data.map((e, i, a) => {
		if(e.constructor == Object){
			this_label = e["x"];
			e = e["y"];
			first_index += 1;
		}
		else
			this_label = label[i-first_index];
		if(i==a.length-1) return {'x': this_label, 'y':e}; 
		let prev = a[i - 1];
		let next = a[i + 1];
		if (e === get_val(prev) && e === get_val(next)) return null;
		return {'x': this_label, 'y':e};
	}).filter(function (el) { return el != null; });
	return _data;
}
function addData(chart, label, data) {
	label.forEach(element => chart.data.labels.push(element));
	chart.data.datasets[0].data = factorData(chart.data.datasets[0].data.concat(data), label)
    chart.update();
}
