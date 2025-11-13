# Smart Trainer Fan Controller – ESP32-S3 (FTMS / DS18B20 / High-Power PWM)

Dieses Projekt ist ein intelligenter, leistungsstarker Lüfter-Controller für Rollentrainer (z. B. Tacx Flux), der die Geschwindigkeit über **Bluetooth FTMS** ausliest und damit einen oder mehrere Hochleistungslüfter regelt.  
Die Steuerung basiert auf einem **ESP32-S3**, misst Temperaturen mit zwei DS18B20-Sensoren und treibt große 12-V-Lüfter mit MOSFET-PWM an.

---

## 🚴‍♂️ Funktionsumfang (Aktueller Stand)

- Liest **Indoor Bike Data** via **BLE FTMS** (Geschwindigkeit / Kadenz)
- Berechnet eine dynamische Lüfterregelung abhängig von:
  - Geschwindigkeit (km/h)
  - Temperatur im Raum
  - Temperatur am Lüfter (Heatsink)
  - Benutzerparametern (Potentiometer „alpha“)
- Temperaturmessung über **2× DS18B20**:
  - **Room Sensor** – Umgebungstemperatur
  - **Fan Sensor** – Temperatur am MOSFET-Kühlkörper
- Komfortregelung mit Sicherheitsgrenzen:
  - Ab Soft-Limit → Leistung reduziert
  - Ab Hard-Limit → Lüfter abgeschaltet
- PWM-Ansteuerung: **25 kHz**, **10-Bit Auflösung**
- Stabile, FreeRTOS-basierte Architektur mit drei Tasks:
  - **BLE Task**
  - **Control Task**
  - **Telemetry Task**
- Ausgabe aller Messwerte über Serial-Telemetrie

---

## 🛠️ Geplante Erweiterungen (Coming Soon)

- **leistungsabhängige Lüfterregelung**  
  z. B. abhängig von realer Leistung (Watt), statt nur Geschwindigkeit

- **Kadenzabhängige RGB-LED-Anzeige**  
  - Grün → z. B. „optimale Kadenz“ (80 rpm)  
  - Gelb → Warnbereich  
  - Rot → „über Zielbereich“ (95 rpm oder mehr)  
  - LED-Bar oder RGB-Ring  
  - Vollständig konfigurierbar

- Web-UI (optional) zur Echtzeitkonfiguration (Alpha, Limits, LED-Zonen)

---

## 🔧 Hardware

### **Mikrocontroller**
- **ESP32-S3**
  - BLE 5.0 (NimBLE)
  - FreeRTOS
  - PWM über LEDC (25 kHz)

### **Temperatursensoren**
- **2× DS18B20**
  - Auf demselben OneWire-Bus
  - Messen:
    - Raumtemperatur
    - Lüfter-/Heatsink-Temperatur

### **Stromversorgung**
- **12 V Schaltnetzteil, 30 A**  
  Leistungsreserve für kräftige Lüfter (~80 W)

### **Leistungs-PWM Stufe**
- **MOSFET:** *Infineon IPA040N06NM5SXKSA1*  
  - Rds_on extrem niedrig  
  - Auf großem Kühlkörper montiert (2,60 °C/W)

- **Freilaufdiode:**  
  **Wolfspeed SiC Dual-Schottkydiode (1200 V, 43 A, TO-247)**  
  - Praktisch unzerstörbar  
  - Extrem schnelle Recovery  
  - Wird nur leicht warm  
  - Schutz gegen hohe Induktionsspannungen bei kleinen Duty Cycles

- **PWM:**  
  - 25 kHz (lüfterfreundlich, nicht hörbar)  
  - 10-Bit Resolution

### **Anschlüsse**
- **SP17 3-Pin Stecker** für Lüfter (robust, verriegelbar)

---

## 🔌 Schaltungskonzept (Kurzfassung)

