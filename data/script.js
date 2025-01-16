const ws = new WebSocket(`ws://${window.location.host}/ws`);

let isRecording = false;

// WebSocket-Nachricht empfangen
ws.onmessage = (event) => {
  const data = JSON.parse(event.data);

  if (data.status) {
    isRecording = data.status === "recording";
    updateUI();
  }

  if (data.euler) {
    document.getElementById("euler").innerText = data.euler.join(", ");
  }

  if (data.acceleration) {
    document.getElementById("acceleration").innerText = data.acceleration.join(", ");
  }
};

// Start- und Stop-Befehle senden
function startRecording() {
  ws.send("start");
}

function stopRecording() {
  ws.send("stop");
}

// UI aktualisieren
function updateUI() {
  const statusElement = document.getElementById("status");
  statusElement.innerText = isRecording ? "Recording" : "Idle";
  statusElement.style.color = isRecording ? "green" : "red";
}
