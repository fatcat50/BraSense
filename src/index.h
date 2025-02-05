#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>ESP32 WebSocket Server</title>
    <style>
    html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
    body{margin-top: 50px;}
    h1{color: #444444;margin: 50px auto;}
    p{font-size: 19px;color: #888;}
    #state{font-weight: bold;color: #444;}
    .switch{margin:25px auto;width:80px}
    .toggle{display:none}
    .toggle+label{display:block;position:relative;cursor:pointer;outline:0;user-select:none;padding:2px;width:80px;height:40px;background-color:#ddd;border-radius:40px}
    .toggle+label:before,.toggle+label:after{display:block;position:absolute;top:1px;left:1px;bottom:1px;content:""}
    .toggle+label:before{right:1px;background-color:#f1f1f1;border-radius:40px;transition:background .4s}
    .toggle+label:after{width:40px;background-color:#fff;border-radius:20px;box-shadow:0 2px 5px rgba(0,0,0,.3);transition:margin .4s}
    .toggle:checked+label:before{background-color:#4285f4}
    .toggle:checked+label:after{margin-left:42px}
    </style>
  </head>
  <body>
    <h1>ESP32 WebSocket Server</h1>
    <div class="switch">
      <input id="toggle-btn" class="toggle" type="checkbox" %CHECK%>
      <label for="toggle-btn"></label>
    </div>
    <p>On-board LED: <span id="state">%STATE%</span></p>

    <script>
	  window.addEventListener('load', function() {
		var websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
		websocket.onopen = function(event) {
		  console.log('Connection established');
		}
		websocket.onclose = function(event) {
		  console.log('Connection died');
		}
		websocket.onerror = function(error) {
		  console.log('error');
		};
		websocket.onmessage = function(event) {
		  if (event.data == "1") {
			document.getElementById('state').innerHTML = "GESTARTET";
			document.getElementById('toggle-btn').checked = true;
		  }
		  else {
			document.getElementById('state').innerHTML = "GESTOPPT";
			document.getElementById('toggle-btn').checked = false;
		  }
		};
		
		document.getElementById('toggle-btn').addEventListener('change', function() { websocket.send('toggle'); });
	  });
	</script>
  </body>
</html>
)rawliteral";

#endif // INDEX_H