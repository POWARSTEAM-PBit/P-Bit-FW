# 🧠 Legacy Firmware Archive – P-Bit

This folder contains **archived versions of P-Bit firmware**, tools for flashing the ESP32, and experimental builds used for calibration and testing.

---

## 🔖 Latest Stable Firmware

📌 **Version: P-BitFW05**
Located in: `/P-BitFW05/`

This version includes:

* `FW.ino.bin`
* `FW.ino.bootloader.bin`
* `FW.ino.partitions.bin`

Use the **flash\_download\_tool\_3.9.7** provided in this folder to upload these binaries to a P-Bit device.

---

## 🔧 Flashing Instructions

To flash a `.bin` firmware to the P-Bit:

1. Open `flash_download_tool_3.9.7/flash_download_tool_3.9.7.exe`
2. Select `ESP32` chip type
3. Load the three `.bin` files with their correct memory addresses:

   * `bootloader.bin` → 0x1000
   * `FW.ino.bin` → 0x10000
   * `partitions.bin` → 0x8000
4. Connect the P-Bit via USB and select the appropriate COM port
5. Click **Start** to begin flashing

---

## 🎧 Audio Calibration Firmware

You will also find a custom firmware build used to calibrate the P-Bit's sound sensor, located in `/AudioCalibration/`.

⚠ This firmware is intended for testing environments only, not for classroom deployment.

---

## 📁 Additional Versions

Other versions included:

* `P-BitFW01` to `P-BitFW04` (historical builds)
* `P-BitFW.ino.zip` – early generic version

---

## 📌 Notes

* All `.zip` files contain either compiled binaries or `.ino` source code.
* Make sure to select the correct version for your needs.
* `User_Setup.h` files have been moved to `legacy/arduino(ino)/`, where they belong.

---

© POWAR STEAM SL – For educational and non-commercial use only.
