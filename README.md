
# Smart Wristband for Laborers and Athletes

An IoT-based wearable device designed to continuously monitor vital signs (heart rate, SpO₂, and body temperature) for individuals working in physically demanding environments, such as construction workers and athletes. The wristband provides real-time health monitoring with alerts via a vibration motor and a remote web-based dashboard.

## 📌 Project Overview

This project was developed as part of the **Internet of Things (EE 402040)** course at the **University of Vocational Technology** by Mechatronics Technology undergraduates. It combines embedded systems, sensor interfacing, wireless communication, and web development to provide a cost-effective, practical solution for real-time health monitoring.

## 👥 Team Members

- **K.P.G. Nirasha Buddhimali** – MEC/22/B1/17  
- **A.D.C. Jayamandira** – MEC/22/B1/45

Supervisor: **Mr. Janith Kasun**

## 🎯 Project Aim

To design and implement a smart wristband that:
- Continuously monitors heart rate, blood oxygen level, and body temperature.
- Sends real-time health data to a web application.
- Provides haptic feedback for alerts when abnormal readings are detected.

## ✅ Features

- Real-time monitoring via embedded sensors (MAX30100 & DS18B20)
- Vibration alerts for out-of-range values
- HTTP communication using ESP32 Wi-Fi
- Python-based web app for live dashboard
- JSON-based data transfer
- User-configurable alert thresholds

## 🖼️ System Architecture

![System Architecture](path-to-system-architecture-image)

## 🧰 Technologies Used

### Hardware:
- **ESP32-WROOM-32** (Microcontroller)
- **MAX30100** (Heart Rate & SpO₂ Sensor)
- **DS18B20** (Digital Temperature Sensor)
- **Vibration Motor** (Haptic Alerts)
- **LiPo Battery (3.7V 150mAh)**
- Additional: Resistors, Breadboard, Charging Module (TP4056)

### Software:
- **ESP32 Firmware** – C++ (Arduino IDE)
- **Web Server** – Python (Flask), HTML/CSS
- **Wiring/Circuit Diagrams** – KiCAD, Fritzing
- **Prototyping** – TinkerCAD

## 📦 Project Structure

```
SmartWristband/
├── main.ino                # ESP32 C++ code
├── circuit/                # Circuit diagrams (PDF/Image)
├── docs/                   # Report, Presentation 
├── images/                 # Photos of prototype & UI
└── README.md               # Project overview
```

## 🔧 Setup Instructions

### ESP32 Firmware
1. Open `firmware/main.cpp` in Arduino IDE.
2. Install necessary libraries: `WiFi.h`, `HTTPClient.h`, `OneWire.h`, `DallasTemperature.h`, etc.
3. Flash code to ESP32 via USB.
4. Update Wi-Fi credentials in code before flashing.

### Web App
1. Navigate to `web-app/`.
2. Install dependencies:
   ```bash
   pip install flask
   ```
3. Run the app:
   ```bash
   python app.py
   ```
4. Access the dashboard at: `http://localhost:5000/`

## 📊 Thresholds & Alerts

- **Heart Rate:** > 130 bpm triggers alert
- **SpO₂ Level:** < 90% triggers alert
- **Temperature:** > 38.0°C triggers alert

Alerts are sent both via vibration motor and to the web UI.

## 🧪 Project Outcomes

- Successfully integrated multiple sensors on a wearable device
- Built a responsive real-time monitoring dashboard
- Demonstrated early warning capability through alerts
- Delivered a full-stack IoT solution from hardware to web

## 📚 References

See full list of academic references and datasheets in `docs/Final_Project_Report.pdf`.

## 📄 License

This project is for educational purposes under academic supervision. Contact the authors for reuse or collaboration.
