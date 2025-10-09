# 🌐 P-Bit WebApp Demo – BLE Dashboard

This folder contains a **demo version** of a web-based dashboard that connects to the P-Bit device using **Bluetooth Low Energy (BLE)**.

It is fully client-side and runs directly in the browser to receive, visualize, and export environmental data collected by the P-Bit.

---

## 🔗 How It Works

1. Flash the P-Bit with this firmware:  
   `/legacy/arduino(ino)/Test Codes/PBIT_BLE_JSON_WORKING/`

2. Open `index.html` in **Google Chrome** (or another BLE-compatible browser).

3. Click **“Connect P-Bit”** and select your device from the list.

4. The dashboard will display:
   - 🌡️ Air temperature
   - 💧 Air humidity
   - 📈 Real-time graphs
   - 🧾 Connection log

---

## ⚙️ Features

- Real-time BLE data visualization
- Chart.js for interactive graphs
- Manual reconnection & status indicators
- Sensor data export as CSV
- Connection log export
- Observation notes section

---

## 📁 Files

- `index.html` – Main dashboard page
- `scripts.js` – BLE logic and chart handling
- `styles.css` – Custom styling (colors, layout)
- `registro_datos.txt` – Sample output (optional log file)
- `/assets/logo.png` – POWAR STEAM logo

---

## 🚧 Status

This version is a **prototype** created for internal testing and educational demo purposes only.

A more polished, multilingual and gamified platform will be developed during the RMIT collaboration.

---

## 🧠 Requirements

- Works best in **Google Chrome** on desktop
- P-Bit must advertise sensor data via BLE in JSON format
- Browser permissions for Bluetooth access are required

---

© POWAR STEAM SL – For educational and non-commercial use only.
