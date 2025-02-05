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
    <p>Messung: <span id="state">%STATE%</span></p>

    <canvas id="chart"></canvas>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

    <script>
    var websocket;

    window.addEventListener('load', function() {
        websocket = new WebSocket(`ws://${window.location.hostname}/ws`);

        websocket.onopen = function(event) {
            console.log('WebSocket verbunden');
        };

        websocket.onclose = function(event) {
            console.log('WebSocket getrennt');
        };

        websocket.onerror = function(error) {
            console.log('WebSocket Fehler:', error);
        };

        websocket.onmessage = function(event) {
            console.log("WebSocket Nachricht:", event.data);

            try {
                let data = JSON.parse(event.data);

                if (data.hasOwnProperty("x") && data.hasOwnProperty("y") && data.hasOwnProperty("z")) {
                    updateChart(data.x, data.y, data.z);
                } else if (event.data == "1") {
                    document.getElementById('state').innerHTML = "GESTARTET";
                    document.getElementById('toggle-btn').checked = true;
                } else if (event.data == "0") {
                    document.getElementById('state').innerHTML = "GESTOPPT";
                    document.getElementById('toggle-btn').checked = false;
                }
            } catch (e) {
                console.log("⚠️ Fehler beim Verarbeiten der WebSocket-Daten:", e);
            }
        };

        document.getElementById('toggle-btn').addEventListener('change', function() { 
            websocket.send('toggle'); 
        });
    });

    let ctx = document.getElementById('chart').getContext('2d');
    let chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
              { label: 'X-Achse', borderColor: 'red', data: [], fill: false },
              { label: 'Y-Achse', borderColor: 'green', data: [], fill: false },
              { label: 'Z-Achse', borderColor: 'blue', data: [], fill: false }
            ]
        },
        options: {
            responsive: true,
            scales: {
                x: { type: 'linear', position: 'bottom' },
                y: { beginAtZero: false }
            }
        }
    });

    function updateChart(x, y, z) {
        let timestamp = Date.now();

        if (chart.data.labels.length > 20) {
            chart.data.labels.shift();
            chart.data.datasets[0].data.shift();
            chart.data.datasets[1].data.shift();
            chart.data.datasets[2].data.shift();
        }

        chart.data.labels.push(timestamp);
        chart.data.datasets[0].data.push({ x: timestamp, y: x });
        chart.data.datasets[1].data.push({ x: timestamp, y: y });
        chart.data.datasets[2].data.push({ x: timestamp, y: z });
        chart.update();
    }
</script>
  </body>
</html>
)rawliteral";

#endif // INDEX_H