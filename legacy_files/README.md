# 🌱 POWAR STEAM – P-Bit Firmware & Dashboard

**P-Bit** is an educational environmental microcomputer developed by [POWAR STEAM SL](https://www.powarsteam.com), designed to help students understand sustainability, climate change, and environmental monitoring using tangible STEAM technology.

This repository serves as a collaboration hub for development of:
- 🔧 New firmware versions
- 🌐 BLE-based WebApp dashboards
- 🧪 Experimental sensor integrations

---

## 🔍 What is the P-Bit?

The **P-Bit** is a compact, low-voltage data logger based on an ESP32. It features:

- 🌡️ Air temperature & humidity sensors (DHT11, DS18B20)  
- ☀️ Light sensor (LDR)  
- 🔊 Analog sound sensor  
- 🌱 Soil temperature & moisture probes  
- 🎛️ TFT screen + rotary encoder for navigation  
- 💡 RGB LED + buzzer for feedback  
- 📶 BLE for wireless data transfer  

It’s designed for **K-12 students** to explore science, technology, and sustainability through real-world data and experiments.

---

## 📁 Repository Structure

```bash
.
├── Arduino(ino)/             # Final .ino source code for current and stable firmware
├── dashboard/                # Placeholder – future WebApp version (under RMIT development)
├── firmware/                 # Placeholder – production firmware will be added here
├── legacy/                   # Archive of older or experimental firmware and WebApps
│   ├── arduino(ino)/         # Experimental Arduino source code and test sketches
│   ├── firmware/             # Compiled binaries, flasher tool, audio calibration firmwares
│   └── P-Bit_WebApp/         # Working BLE-connected WebApp (demo version)
└── assets/                   # Logos and shared assets
```

---

## 🚀 Getting Started

> *(Detailed instructions will be added once the RMIT versions are merged into `main`)*

For now:
- To test BLE communication, use the firmware in `legacy/arduino(ino)/Test Codes/PBIT_BLE_JSON_WORKING`
- Pair it with the WebApp in `legacy/P-Bit_WebApp/` by opening `index.html` in Chrome

---

## 📡 BLE Data Format

Sensor data is transmitted from the P-Bit using this JSON structure:

```json
{
  "device": "PBIT",
  "air_temp": 31.5,
  "air_hum": 55.0,
  "light": 1234,
  "noise": 302,
  "soil_temp": 29.1,
  "soil_hum": 78
}
```

This ensures consistency across firmware and dashboard development.

---

## 🧠 Collaboration Guidelines

- RMIT students and collaborators: please work in your own branch  
- Merge into `main` only after internal testing and code review  
- Follow the folder structure conventions  
- Document changes clearly in commit messages  

---

## 🎓 Educational Use Only

This project is released for **educational and non-commercial** purposes.

Educators and students are encouraged to:
- Use it in science & STEAM classrooms  
- Extend or adapt it for experiments  
- Share derivatives under the same license, with attribution  

🔗 Learn more about the project:  
https://www.powarsteam.es/en/pbit-environmental-mini-computer

🎥 Watch a demo video:  
https://www.youtube.com/watch?v=ga6qz10fXHc

📧 Contact: [info@powarsteam.com](mailto:info@powarsteam.com)

---

## 🔐 License

This project is licensed under:

> **Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International**  
> (CC BY-NC-SA 4.0)

- ✅ Use for educational purposes  
- ❌ Commercial use prohibited  
- 🔄 Must share under same terms  
- 🧾 Attribution to POWAR STEAM SL required  

See [`LICENSE`](./LICENSE) for full terms.

---

## 🧑‍🏫 About POWAR STEAM

POWAR STEAM SL is a social-impact edtech startup based in Barcelona, Spain. We design and manufacture educational tools that combine design, science, and technology to help children explore the natural world and become **environmental changemakers**.

🌍 [www.powarsteam.com](https://www.powarsteam.com)
