# 🧪 Arduino Source Code – Experimental Firmware

This folder contains source code files (`.ino`) for multiple development and test versions of the P-Bit firmware.

It is intended for developers or collaborators working on sensor integration, visualization improvements, and new firmware features.

---

## 🛠 Folder Contents

- `/P_Bit_Full_2024_04_17/`  
  → General firmware version that **reads sensors and displays data on the TFT screen**.  
  This version does **not use BLE**, but is the most complete implementation for offline experiments.

- `/FW_modified_DS18B20_v2/`  
  → Experimental version using alternative DS18B20 soil sensor logic.

- `/Test Codes/PBIT_BLE_JSON_WORKING/`  
  → Minimal BLE prototype. This version **sends sensor data via BLE in JSON format**, compatible with the WebApp demo.  
  ⚠ It does **not display anything on the screen**.

- `/Test Codes/`  
  → Folder reserved for sensor-specific or feature test sketches.  
  🔧 A dedicated README for this folder will be created once all files are consolidated and documented.

- `/LIBRARIES/`  
  → Required libraries in `.zip` format. Install them via Arduino IDE or PlatformIO.

---

## ⚙️ TFT Display Configuration

Each sketch must include a working `User_Setup.h` file to define the correct screen settings.

👉 Make sure to place the correct `User_Setup.h` in the **same folder** as the `.ino` file before compiling.

- You will find working examples in:
  - `/P_Bit_Full_2024_04_17/User_Setup.h`
  - `User_Setup.h` at root (fallback configuration)
  - `(Original)User_Setup.h` (as backup or default)

---

## 🧩 Notes

- These versions are **not final releases** and may be unstable or incomplete.
- This folder is part of the **legacy archive** and will be replaced by stable versions in the main branch when ready.
- Contributions and branches should reference these files only for prototyping or testing.

---

© POWAR STEAM SL – For educational and non-commercial use only.
