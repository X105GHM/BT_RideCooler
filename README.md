# Smart Trainer Fan Controller – ESP32-S3 (FTMS / DS18B20 / High-Power PWM)

This project is a high-performance, intelligent fan controller for smart bike trainers (e.g., Tacx Flux).  
It reads **speed/cadence via Bluetooth FTMS** and controls one or more powerful 12-V fans using PWM.  
The system is built around an **ESP32-S3**, uses two DS18B20 temperature sensors, and drives high-current fans using a MOSFET power stage.

---

## 🚴‍♂️ Current Features

- Reads **Indoor Bike Data** via **BLE FTMS**
- Dynamic fan control based on:
  - Trainer speed (km/h)
  - Room temperature (DS18B20 #1)
  - Heatsink temperature at the MOSFET / fan assembly (DS18B20 #2)
  - User control via potentiometer ("alpha")
- Safety-aware thermal control:
  - **Soft limit:** reduce fan output gradually  
  - **Hard limit:** immediately shut down fans
- High sampling frequency: control loop every 20 ms
- Smooth PWM control at **25 kHz**, **10-bit resolution**
- FreeRTOS-based software architecture:
  - BLE Task  
  - Control Task  
  - Telemetry Task
- Serial telemetry output (speed, temperatures, alpha, duty, etc.)

---

## 🛠️ Planned Features (Coming Soon)

- **Power-based fan control**  
  Instead of speed only, future versions will use trainer watt output.

- **Cadence-dependent RGB LED indicator**
  - Green → optimal cadence (e.g., 80 rpm)
  - Yellow → moderate zone
  - Red → above threshold (e.g., 95 rpm)
  - Fully configurable ranges
  - Based on WS2812 or similar

- Optional Web-UI configuration (alpha, thresholds, LED zones, fan curves)

---

## 🔧 Hardware Overview

### **Microcontroller**
- **ESP32-S3**
  - BLE 5 (NimBLE)
  - FreeRTOS multitasking
  - LEDC PWM (25 kHz)

### **Temperature Sensors**
- **2× DS18B20**
  - On the same OneWire bus
  - **Room Sensor** → measures ambient temperature  
  - **Fan/Heatsink Sensor** → mounted on the MOSFET heatsink

### **Power Supply**
- **12 V / 30 A SMPS**
  - Provides enough power for high-performance fans (~80 W)

### **PWM Power Stage**
- **MOSFET:** *Infineon IPA040N06NM5SXKSA1*
  - Very low Rds_on
  - Mounted on a large heatsink (2.60 °C/W)

- **Freewheeling Diode:**  
  **Wolfspeed SiC Dual Schottky Diode (1200 V, 43 A, TO-247)**  
  - Handles extremely high inductive kickback  
  - Essential at low PWM duty cycles  
  - Runs only mildly warm during operation  
  - Virtually indestructible

- **PWM Specs**
  - 25 kHz switching frequency (inaudible)
  - 10-bit resolution

### **Connectors**
- **SP17 3-pin waterproof connector** for the fan output

---

## 🧩 System Architecture

