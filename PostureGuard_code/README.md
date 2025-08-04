## Code Overview 🔍

- **WiFi Setup** 📶  
  Establishes a WiFi connection using either stored credentials or via the WiFiManager access point for easy configuration.

- **MQTT Client Initialization** 🔌  
  Connects securely to the HiveMQ Cloud MQTT broker to publish posture alerts.

- **MPU9250 Sensor Initialization** 🎛️  
  Configures and calibrates the MPU9250 IMU sensor to measure body tilt angles accurately.

- **Posture Monitoring Logic** 🧍‍♂️  
  Continuously reads the sensor’s tilt angle and detects incorrect posture based on a predefined threshold (angle < 70°).  
  - Triggers buzzer alerts when poor posture is detected for a sustained period.  
  - Sends posture status messages via MQTT.  
  - Tracks the number of posture deviations.

- **Buzzer Control** 🔔  
  Activates or deactivates the buzzer based on posture status.

- **Movement Detection and Idle Alert** 🎢  
  Monitors gyroscope data to detect user movement and sends an idle alert if no significant movement is detected for a specified duration.

- **MQTT Messaging** 💬  
  Sends JSON-formatted alerts containing posture information to the designated MQTT topic.

