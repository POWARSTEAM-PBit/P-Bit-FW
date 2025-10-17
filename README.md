# 🧠 P-Bit Firmware

This repository contains the firmware for the P-Bit IoT board, powered by an ESP32.
It manages sensor input, TFT display output, BLE communication, and rotary encoder interaction — designed for modular, real-time environmental sensing and display.



## Getting Started

Firstly clone the repository:

```
git clone git@github.com:POWARSTEAM-PBit/P-Bit-FW.git
```

To build the following firmware:

```
platformio run
```

To build and upload the firmware:

```
platformio run -t upload
```

To monitor the serial output:

```
platformio device monitor
```