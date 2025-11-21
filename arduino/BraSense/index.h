#ifndef INDEX_H 
#define INDEX_H 

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1.0, user-scalable=no"
    />
    <title>ESP32 WebSocket Server</title>
    <style>
      html {
        font-family: Helvetica;
        display: inline-block;
        margin: 0px auto;
        text-align: center;
      }
      body {
        margin-top: 20px;
      }
      h1 {
        display: none;
      }
      p {
        font-size: 19px;
        color: #888;
      }
      #state {
        font-weight: bold;
        color: #444;
      }
      .switch {
        margin: 10px auto;
        width: 80px;
      }
      .toggle {
        display: none;
      }
      .toggle + label {
        display: block;
        position: relative;
        cursor: pointer;
        outline: 0;
        user-select: none;
        padding: 2px;
        width: 80px;
        height: 40px;
        background-color: #ddd;
        border-radius: 40px;
      }
      .toggle + label:before,
      .toggle + label:after {
        display: block;
        position: absolute;
        top: 1px;
        left: 1px;
        bottom: 1px;
        content: "";
      }
      .toggle + label:before {
        right: 1px;
        background-color: #f1f1f1;
        border-radius: 40px;
        transition: background 0.4s;
      }
      .toggle + label:after {
        width: 40px;
        background-color: #fff;
        border-radius: 20px;
        box-shadow: 0 2px 5px rgba(0, 0, 0, 0.3);
        transition: margin 0.4s;
      }
      .toggle:checked + label:before {
        background-color: #4285f4;
      }
      .toggle:checked + label:after {
        margin-left: 42px;
      }
    </style>
  </head>
  <body>
    <p id="state" style="font-weight: bold; margin-bottom: 5px">
      State: <span>%STATE%</span>
    </p>

    <div class="switch">
      <input id="toggle-btn" class="toggle" type="checkbox" %CHECK% />
      <label for="toggle-btn"></label>
    </div>

    <button id="download-btn" onclick="downloadAndConvert()">
      Download Log-File
    </button>

    <div id="charts-container"></div>

    <script src="https://cdn.jsdelivr.net/npm/chart.js/dist/chart.umd.min.js"></script>

    <script>
      var websocket;
      let charts = [];
      let chartCounter = 0;
      let startTime = null; // Startzeit erst setzen, wenn Messung startet
      let isMeasuring = false;

      window.addEventListener("load", function () {
        websocket = new WebSocket(`ws://${window.location.hostname}/ws`);

        websocket.onopen = function () {
          console.log("WebSocket connected");
        };

        websocket.onclose = function () {
          console.log("WebSocket disconnected");
        };

        websocket.onerror = function (error) {
          console.log("WebSocket Error:", error);
        };

        websocket.onmessage = function (event) {
          console.log("WebSocket Data:", event.data);

          // 1) Erst Steuer-Messages "1"/"0" behandeln
          if (event.data === "1") {
            setMeasuringUI(true);
            return;
          }
          if (event.data === "0") {
            setMeasuringUI(false);
            return;
          }

          // 2) Alles andere versuchen als JSON (Sensordaten)
          try {
            let data = JSON.parse(event.data);

            if (
              data.hasOwnProperty("x") &&
              data.hasOwnProperty("y") &&
              data.hasOwnProperty("z")
            ) {
              if (isMeasuring) {
                updateCurrentChart(data.x, data.y, data.z);
              }
            }
          } catch (e) {
            console.log("Fehler beim Verarbeiten der WebSocket-Daten:", e);
          }
        };

        document
          .getElementById("toggle-btn")
          .addEventListener("change", function () {
            websocket.send("toggle");
          });
      });

      // UI bei Start/Stop der Messung umschalten
      function setMeasuringUI(running) {
        isMeasuring = running;

        document.getElementById("state").innerHTML =
          "State: " + (running ? "Measuring..." : "Standby");
        document.getElementById("toggle-btn").checked = running;

        // Download-Button ein/aus
        const dlBtn = document.getElementById("download-btn");
        if (dlBtn) dlBtn.disabled = running;

        if (running) {
          startMeasurement();
        } else {
          stopMeasurement();
        }
      }

      // Binär -> CSV im Browser
      async function downloadAndConvert() {
        try {
          // 1. Binärdaten vom ESP holen
          const response = await fetch("/downloadBin");
          if (!response.ok) {
            alert("Fehler beim Download: " + response.status);
            return;
          }

          const buffer = await response.arrayBuffer();

          // => 4 Floats (time, x, y, z), Little-Endian
          const recordSize = 4 * 4; // 4 floats * 4 bytes
          const view = new DataView(buffer);
          const numRecords = Math.floor(buffer.byteLength / recordSize);

          console.log("Bytes:", buffer.byteLength, "Records:", numRecords);

          const lines = [];
          lines.push("Time [s];X [deg];Y [deg];Z [deg]");

          for (let i = 0; i < numRecords; i++) {
            const offset = i * recordSize;

            const time = view.getFloat32(offset + 0, true); // little endian
            const x = view.getFloat32(offset + 4, true);
            const y = view.getFloat32(offset + 8, true);
            const z = view.getFloat32(offset + 12, true);

            const tStr = time.toFixed(3).replace(".", ",");
            const xStr = x.toFixed(2).replace(".", ",");
            const yStr = y.toFixed(2).replace(".", ",");
            const zStr = z.toFixed(2).replace(".", ",");

            lines.push(`${tStr};${xStr};${yStr};${zStr}`);
          }

          const csvContent = lines.join("\r\n");

          // 3. CSV-Download im Browser auslösen
          const blob = new Blob([csvContent], {
            type: "text/csv;charset=utf-8;",
          });
          const url = URL.createObjectURL(blob);

          const a = document.createElement("a");
          a.href = url;
          a.download = "converted_data.csv";
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);
        } catch (err) {
          console.error(err);
          alert("Fehler bei Download/Konvertierung (Details in der Konsole).");
        }
      }

      function startMeasurement() {
        isMeasuring = true;
        startTime = Date.now() / 1000;
        createNewChart();
      }

      function stopMeasurement() {
        isMeasuring = false;
      }

      function createNewChart() {
        chartCounter++;

        let chartContainer = document.createElement("div");
        chartContainer.style.textAlign = "center";
        chartContainer.style.marginBottom = "40px";

        let chartTitle = document.createElement("h3");
        chartTitle.innerText = "Measurement #" + chartCounter;
        chartTitle.style.marginBottom = "5px";

        let canvas = document.createElement("canvas");
        canvas.id = "chart" + chartCounter;
        canvas.style.marginTop = "10px";

        chartContainer.appendChild(chartTitle);
        chartContainer.appendChild(canvas);

        let chartsContainer = document.getElementById("charts-container");
        chartsContainer.insertBefore(
          chartContainer,
          chartsContainer.firstChild
        );

        let ctx = canvas.getContext("2d");
        let newChart = new Chart(ctx, {
          type: "line",
          data: {
            labels: [],
            datasets: [
              { label: "X-Axis", borderColor: "red", data: [], fill: false },
              { label: "Y-Axis", borderColor: "green", data: [], fill: false },
              { label: "Z-Axis", borderColor: "blue", data: [], fill: false },
            ],
          },
          options: {
            responsive: true,
            scales: {
              x: { type: "linear", position: "bottom" },
              y: { beginAtZero: false },
            },
          },
        });

        charts.push(newChart);
      }

      function updateCurrentChart(x, y, z) {
        if (charts.length === 0 || !isMeasuring) return; // Falls keine Messung läuft, nichts tun

        let currentChart = charts[charts.length - 1];
        let timestamp = Date.now() / 1000 - startTime;

        if (currentChart.data.labels.length > 300) {
          currentChart.data.labels.shift();
          currentChart.data.datasets[0].data.shift();
          currentChart.data.datasets[1].data.shift();
          currentChart.data.datasets[2].data.shift();
        }

        currentChart.data.labels.push(timestamp);
        currentChart.data.datasets[0].data.push({ x: timestamp, y: x });
        currentChart.data.datasets[1].data.push({ x: timestamp, y: y });
        currentChart.data.datasets[2].data.push({ x: timestamp, y: z });
        currentChart.update("none");
      }
    </script>
  </body>
</html>
)rawliteral"; 
#endif // INDEX_H