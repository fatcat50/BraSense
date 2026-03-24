#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no"/>
    <title>BraSense – Dual MTi</title>
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0 auto; text-align: center; }
      body { margin-top: 20px; }
      h1   { display: none; }
      p    { font-size: 19px; color: #888; }
      #state { font-weight: bold; color: #444; }
      .badge {
        display: inline-block; padding: 2px 8px; border-radius: 10px;
        font-size: 12px; font-weight: bold; margin-left: 6px; color: #fff;
      }
      .badge-single { background: #f59e0b; }
      .badge-dual   { background: #10b981; }
      .switch { margin: 10px auto; width: 80px; }
      .toggle { display: none; }
      .toggle + label {
        display: block; position: relative; cursor: pointer;
        outline: 0; user-select: none; padding: 2px;
        width: 80px; height: 40px; background-color: #ddd; border-radius: 40px;
      }
      .toggle + label:before,
      .toggle + label:after {
        display: block; position: absolute; top: 1px; left: 1px; bottom: 1px; content: "";
      }
      .toggle + label:before {
        right: 1px; background-color: #f1f1f1; border-radius: 40px; transition: background 0.4s;
      }
      .toggle + label:after {
        width: 40px; background-color: #fff; border-radius: 20px;
        box-shadow: 0 2px 5px rgba(0,0,0,0.3); transition: margin 0.4s;
      }
      .toggle:checked + label:before { background-color: #4285f4; }
      .toggle:checked + label:after  { margin-left: 42px; }
      button { margin: 4px; padding: 8px 16px; font-size: 14px; cursor: pointer; }
    </style>
  </head>
  <body>
    <p id="state" style="font-weight:bold;margin-bottom:5px">
      State: <span>%STATE%</span>
      <span id="mode-badge" class="badge"></span>
    </p>

    <div class="switch">
      <input id="toggle-btn" class="toggle" type="checkbox" %CHECK% />
      <label for="toggle-btn"></label>
    </div>

    <button id="download-btn" onclick="downloadAndConvert()">Download CSV</button>

    <div id="charts-container"></div>
    <script src="https://cdn.jsdelivr.net/npm/chart.js/dist/chart.umd.min.js"></script>
    <script>
      var websocket;
      let charts      = [];   // [[chartS1, chartS2], ...]
      let chartCounter = 0;
      let startTime    = null;
      let isMeasuring  = false;
      let dualMode     = false;   // wird aus den ersten Daten erkannt

      window.addEventListener("load", function () {
        websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
        websocket.onopen    = () => console.log("WS verbunden");
        websocket.onclose   = () => console.log("WS getrennt");
        websocket.onerror   = (e) => console.log("WS Fehler:", e);

        websocket.onmessage = function (event) {
          if (event.data === "1") { setMeasuringUI(true);  return; }
          if (event.data === "0") { setMeasuringUI(false); return; }

          try {
            let d = JSON.parse(event.data);
            if (isMeasuring && d.hasOwnProperty("x1")) {
              // Dual-Modus erkennen: Sensor 2 sendet Nicht-Null
              if (!dualMode && (d.x2 !== 0 || d.y2 !== 0 || d.z2 !== 0)) {
                dualMode = true;
                document.getElementById("mode-badge").textContent = "DUAL";
                document.getElementById("mode-badge").className   = "badge badge-dual";
              }
              updateCharts(d.x1, d.y1, d.z1, d.x2, d.y2, d.z2);
            }
          } catch(e) { console.log("Parse-Fehler:", e); }
        };

        document.getElementById("toggle-btn")
          .addEventListener("change", () => websocket.send("toggle"));
      });

      function setMeasuringUI(running) {
        isMeasuring = running;
        document.getElementById("state").innerHTML =
          "State: " + (running ? "Measuring..." : "Standby") +
          ` <span id="mode-badge" class="badge ${dualMode ? 'badge-dual' : 'badge-single'}">${dualMode ? 'DUAL' : 'SINGLE'}</span>`;
        document.getElementById("toggle-btn").checked = running;
        document.getElementById("download-btn").disabled = running;
        if (running) { isMeasuring = true; startTime = Date.now() / 1000; createNewCharts(); }
        else          { isMeasuring = false; }
      }

      // ── CSV-Download (7 Floats pro Record: time, x1,y1,z1, x2,y2,z2) ──────
      async function downloadAndConvert() {
        try {
          const response = await fetch("/downloadBin");
          if (!response.ok) { alert("Fehler: " + response.status); return; }

          const buf        = await response.arrayBuffer();
          const recordSize = 7 * 4;   // 7 floats × 4 Bytes = 28 Bytes
          const view       = new DataView(buf);
          const numRecords = Math.floor(buf.byteLength / recordSize);

          console.log(`${buf.byteLength} Bytes → ${numRecords} Datenpunkte`);

          const lines = ["Time [s];X1 [m/s²];Y1 [m/s²];Z1 [m/s²];X2 [m/s²];Y2 [m/s²];Z2 [m/s²]"];
          const fmt   = (v, d=2) => v.toFixed(d).replace(".", ",");

          for (let i = 0; i < numRecords; i++) {
            const o  = i * recordSize;
            const t  = view.getFloat32(o +  0, true);
            const x1 = view.getFloat32(o +  4, true);
            const y1 = view.getFloat32(o +  8, true);
            const z1 = view.getFloat32(o + 12, true);
            const x2 = view.getFloat32(o + 16, true);
            const y2 = view.getFloat32(o + 20, true);
            const z2 = view.getFloat32(o + 24, true);
            lines.push(`${fmt(t,4)};${fmt(x1)};${fmt(y1)};${fmt(z1)};${fmt(x2)};${fmt(y2)};${fmt(z2)}`);
          }

          const blob = new Blob([lines.join("\r\n")], { type: "text/csv;charset=utf-8;" });
          const url  = URL.createObjectURL(blob);
          const a    = Object.assign(document.createElement("a"),
                         { href: url, download: "brasense_data.csv" });
          document.body.appendChild(a); a.click();
          document.body.removeChild(a); URL.revokeObjectURL(url);
        } catch(err) {
          console.error(err);
          alert("Fehler beim Download (Details in der Konsole).");
        }
      }

      // ── Charts: pro Messung je 1 Chart für Sensor 1 und Sensor 2 ──────────
      function createNewCharts() {
        chartCounter++;
        const container = document.getElementById("charts-container");

        const pair = ["Sensor 1", "Sensor 2"].map((label, idx) => {
          const wrap   = document.createElement("div");
          wrap.style.cssText = "text-align:center;margin-bottom:30px";
          const title  = document.createElement("h3");
          title.innerText = `Measurement #${chartCounter} – ${label}`;
          const canvas = document.createElement("canvas");
          canvas.id    = `chart_${chartCounter}_s${idx+1}`;
          wrap.appendChild(title);
          wrap.appendChild(canvas);
          container.insertBefore(wrap, container.firstChild);

          return new Chart(canvas.getContext("2d"), {
            type: "line",
            data: {
              labels: [],
              datasets: [
                { label: "X", borderColor: "red",   data: [], fill: false, pointRadius: 0 },
                { label: "Y", borderColor: "green", data: [], fill: false, pointRadius: 0 },
                { label: "Z", borderColor: "blue",  data: [], fill: false, pointRadius: 0 },
              ],
            },
            options: {
              animation: false,
              responsive: true,
              scales: {
                x: { type: "linear", position: "bottom" },
                y: { beginAtZero: false },
              },
            },
          });
        });

        charts.push(pair);
      }

      function updateCharts(x1,y1,z1, x2,y2,z2) {
        if (charts.length === 0 || !isMeasuring) return;
        const pair = charts[charts.length - 1];
        const t    = Date.now() / 1000 - startTime;

        [[x1,y1,z1], [x2,y2,z2]].forEach((vals, i) => {
          const c = pair[i];
          if (c.data.labels.length > 300) {
            c.data.labels.shift();
            c.data.datasets.forEach(d => d.data.shift());
          }
          c.data.labels.push(t);
          c.data.datasets[0].data.push({ x: t, y: vals[0] });
          c.data.datasets[1].data.push({ x: t, y: vals[1] });
          c.data.datasets[2].data.push({ x: t, y: vals[2] });
          c.update("none");
        });
      }
    </script>
  </body>
</html>
)rawliteral";
#endif // INDEX_H
