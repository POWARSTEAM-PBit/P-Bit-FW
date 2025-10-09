// 📊 Dashboard PBIT - Registro persistente con intervalo configurable, identificación del dispositivo y eventos en CSV (versión JSON extendida)

let chartTemp, chartHum;
let device, server, jsonChar;
let lastDataTime = null;
let autoScrollEnabled = true;
let dataLog = [];
let logEventos = [];
let intentoReconectar = false;
let reconexionIntentos = 0;
let reconexionInterval = null;
let registroInterval = null;
let intervaloMuestreo = 10; // seconds
let deviceName = "PBIT";

let latestData = {
  air_temp: null,
  air_hum: null,
  light: null,
  noise: null,
  soil_temp: null,
  soil_hum: null,
  device: null
};

function actualizarHora() {
  const ahora = new Date();
  document.getElementById("reloj").innerText =
    `⌚ Hora local: ${ahora.toLocaleTimeString()} | 📅 Fecha: ${ahora.toLocaleDateString()}`;
}
setInterval(actualizarHora, 1000);
actualizarHora();

function obtenerFechaActualComoNombre() {
  const ahora = new Date();
  return ahora.toISOString().replace(/[:]/g, "-").replace("T", "_").split(".")[0];
}

function descargarArchivo(nombreBase, extension, contenido, tipoMime = "text/plain") {
  const blob = new Blob([contenido], { type: tipoMime });
  const a = document.createElement("a");
  a.href = URL.createObjectURL(blob);
  a.download = `${nombreBase}_${obtenerFechaActualComoNombre()}.${extension}`;
  a.click();
}

function setEstado(texto) {
  document.getElementById("estado").innerText = texto;
}

function inicializarGraficos() {
  const ctxT = document.getElementById('grafTemp').getContext('2d');
  chartTemp = new Chart(ctxT, {
    type: 'line',
    data: { labels: [], datasets: [{ label: '🌡️ Air Temp (°C)', data: [], borderColor: '#50B1F9', backgroundColor: 'rgba(80,177,249,0.2)', fill: true, tension: 0.3 }] },
    options: { responsive: true, scales: { y: { beginAtZero: false, suggestedMax: 40 } } }
  });

  const ctxH = document.getElementById('grafHum').getContext('2d');
  chartHum = new Chart(ctxH, {
    type: 'bar',
    data: { labels: [], datasets: [{ label: '💧 Air Humidity (%)', data: [], backgroundColor: '#62F304' }] },
    options: { responsive: true, scales: { y: { beginAtZero: true, suggestedMax: 100 } } }
  });
}

async function conectar() {
  try {
    log("🔍 Searching BLE device P-Bit...");
    setEstado("🔍 Searching P-Bit...");

    device = await navigator.bluetooth.requestDevice({ filters: [{ namePrefix: 'PBIT' }], optionalServices: ["0000181a-0000-1000-8000-00805f9b34fb"] });
    deviceName = device.name || "PBIT";
    device.addEventListener('gattserverdisconnected', reconectar);
    registrarEvento("Connection started");
    await conectarDispositivo();
  } catch (err) {
    log("❌ Connection error: " + err.message);
    alert("Connection error: " + err.message);
    setEstado("❌ Connection error");
    registrarEvento("Connection error: " + err.message);
  }
}

async function conectarDispositivo() {
  server = await device.gatt.connect();
  setEstado("✅ Connected, starting logging...");
  registrarEvento("Device connected");
  clearInterval(reconexionInterval);
  reconexionIntentos = 0;

  const service = await server.getPrimaryService("0000181a-0000-1000-8000-00805f9b34fb");
  jsonChar = await service.getCharacteristic("00002a6e-0000-1000-8000-00805f9b34fb");

  await jsonChar.startNotifications();
  jsonChar.addEventListener('characteristicvaluechanged', e => {
    const decoder = new TextDecoder();
    const jsonString = decoder.decode(e.target.value);
    try {
      const data = JSON.parse(jsonString);
      latestData = { ...latestData, ...data };
    } catch (err) {
      log("⚠️ JSON parse error: " + jsonString);
    }
  });

  iniciarRegistro();
}

function iniciarRegistro() {
  if (registroInterval) clearInterval(registroInterval);
  const input = document.getElementById("intervaloInput");
  if (input) intervaloMuestreo = Math.max(1, parseInt(input.value) || 10);
  registroInterval = setInterval(() => {
    const now = new Date().toISOString();
    const row = [now, latestData.air_temp, latestData.air_hum, latestData.light, latestData.noise, latestData.soil_temp, latestData.soil_hum].join(",");
    dataLog.push(row);
    updateTemperature(latestData.air_temp);
    updateHumidity(latestData.air_hum);
    registrarEvento("Data logged");
  }, intervaloMuestreo * 1000);
}

function desconectar() {
  if (device && device.gatt.connected) {
    device.gatt.disconnect();
    registrarEvento("Manual disconnection");
    setEstado("🔴 Disconnected");
    document.getElementById("tempActual").innerText = "NaN";
    document.getElementById("humActual").innerText = "NaN";
  }
}

function reconectar() {
  setEstado("⛔️ Disconnected. Trying to reconnect...");
  registrarEvento("Disconnection detected");

  if (!intentoReconectar) {
    intentoReconectar = true;
    reconexionIntentos = 0;
    reconexionInterval = setInterval(async () => {
      if (reconexionIntentos >= 10) {
        clearInterval(reconexionInterval);
        registrarEvento("Reconnection failed after 10 attempts");
        intentoReconectar = false;
        return;
      }
      reconexionIntentos++;
      try {
        await conectarDispositivo();
        registrarEvento("Reconnected successfully");
        intentoReconectar = false;
      } catch (e) {
        log("❌ Reconnect attempt failed: " + e.message);
      }
    }, 5000);
  }
}

function registrarEvento(texto) {
  const ahora = new Date().toISOString();
  logEventos.push(`${ahora},${texto}`);
}

function log(msg) {
  const logContainer = document.getElementById("logContainer");
  const entry = document.createElement("div");
  entry.textContent = new Date().toLocaleTimeString() + ": " + msg;
  logContainer.appendChild(entry);
  if (autoScrollEnabled) logContainer.scrollTop = logContainer.scrollHeight;
}

function updateTemperature(temp) {
  document.getElementById("tempActual").innerText = `${temp} °C`;
  chartTemp.data.labels.push(new Date().toLocaleTimeString());
  chartTemp.data.datasets[0].data.push(temp);
  if (chartTemp.data.labels.length > 20) {
    chartTemp.data.labels.shift();
    chartTemp.data.datasets[0].data.shift();
  }
  chartTemp.update();
}

function updateHumidity(hum) {
  document.getElementById("humActual").innerText = `${hum} %`;
  chartHum.data.labels.push(new Date().toLocaleTimeString());
  chartHum.data.datasets[0].data.push(hum);
  if (chartHum.data.labels.length > 20) {
    chartHum.data.labels.shift();
    chartHum.data.datasets[0].data.shift();
  }
  chartHum.update();
}

function exportarDatosSensor() {
  const contenido = "timestamp,air_temp,air_hum,light,noise,soil_temp,soil_hum\n" + dataLog.join("\n");
  descargarArchivo(deviceName + "_data", "csv", contenido, "text/csv");
}

function exportarLogEventos() {
  const contenido = "timestamp,event\n" + logEventos.join("\n");
  descargarArchivo(deviceName + "_log", "csv", contenido, "text/csv");
}

inicializarGraficos();
