# 📱 Posture Guard – Flutter App

**Posture Guard** is a Flutter-based mobile application developed by team ignitrix for the SLIoT Competition 2025. It promotes healthy posture habits by receiving real-time sensor data from an ESP32 device and delivering instant feedback to the user.

> ⚠️ This repository contains **only the mobile app source code**. The ESP32 firmware and hardware setup are maintained separately.

GitHub Repo: [`git@github.com:venumigihansa/Posture-Guard.git`](https://github.com/venumigihansa/Posture-Guard)


## 🚀 Features

### 🔴 Real-Time Posture Monitoring
- Live data stream from ESP32 via **MQTT**
- Continuous **angle measurement** using IMU sensor data
- Smart classification: **Good**, **Needs Improvement**, or **Bad Posture**

### 🔔 Smart Alerts & Feedback
- Real-time **visual status indicators**
- Customizable **sound** and **vibration** alerts
- Adjustable **sensitivity thresholds**

### 📊 History & Insights
- Local data persistence for offline use
- **Posture history tracking** and visual trend charts
- Weekly summaries and exercise recommendations

---

## 🔗 Data Communication

- Secure communication over **MQTT**
- Real-time sync with **ESP32 + IMU Sensor**
- Tested with **HiveMQ Cloud** MQTT broker

---

## 🛠️ Setup & Installation

### 📱 Prerequisites
- Flutter SDK (latest stable version)
- MQTT Broker (e.g., [HiveMQ Cloud](https://www.hivemq.com/mqtt-cloud-broker/))
- ESP32 with IMU sensor (hardware not included in this repo)

