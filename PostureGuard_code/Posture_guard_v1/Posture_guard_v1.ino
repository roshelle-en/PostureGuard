#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <MPU9250_WE.h>
#include <Wire.h>
#include <WiFiManager.h> 

#define MPU9250_ADDR 0x68
#define BUZZER_PIN 4  // Alert output pin

unsigned long previousMillis = 0;
const long ring_time = 5000;
bool correct_posture = true;
bool alert_sent = true;
unsigned long time_now = 0;
bool ctsBuzz = false;
const long alert_time = 20000;
unsigned long lastMqttSentTime = 0;  // Timestamp of last MQTT message sent
const long mqttSendInterval = 30000;  // 30 seconds interval for MQTT message
const long wait_time = 5000;
const long ringing_time = 3000;

bool verifyingPosture = false;
unsigned long verifyStartTime = 0;
const unsigned long verifyDuration = 3000;  // 3 seconds


MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);

bool detection_started = false;
unsigned long detection_time = 0;
unsigned long idle_alert_time = 1 * 60 * 1000;
bool idle_alert_sent = false;

int slouchCount = 0;

// WiFi credentials
const char* ssid = "OnePlus 6T";
const char* password = "12345678";

// HiveMQ Cloud MQTT credentials
const char* mqtt_server = "218638469dfa429db85a2e1df0b4f8c7.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // Secure MQTT port
const char* mqtt_user = "hivemq.webclient.1742574310428";
const char* mqtt_password = "61b!5CgSPQc>xqu.3@JM";
const char* topic = "alert/posture"; // MQTT topic

WiFiClientSecure espClient;
PubSubClient client(espClient);


void connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" Connected!");
}

void connectMQTT() {
    client.setServer(mqtt_server, mqtt_port);
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println(" Connected!");
        } else {
            Serial.print(" Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}




void setup() {
    delay(500);
    Serial.begin(115200);
    Wire.begin(21, 22); // Explicitly set SDA=21, SCL=22
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Initialize alert pin to LOW

    // connectWiFi();
    // espClient.setInsecure();
    connectWithWiFiManager();

    connectMQTT();
    delay(2000);
    // Initialize MPU9250
    if (!myMPU9250.init()) {
      Serial.println("MPU9250 not detected! Continuing without MPU...");
    } else {
      Serial.println("MPU9250 detected!");
    }
    Serial.println("Calibrating MPU9250... Keep the sensor flat.");
    delay(2000);
    myMPU9250.autoOffsets();
    Serial.println("Calibration Done!");

    myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
    myMPU9250.enableAccDLPF(true);
    myMPU9250.setAccDLPF(MPU9250_DLPF_6);
    
    myMPU9250.setGyrRange(MPU9250_GYRO_RANGE_250);  // Degrees/sec
    myMPU9250.enableGyrDLPF();                      // Enable low-pass filter
    myMPU9250.setGyrDLPF(MPU9250_DLPF_6);           // Smooth readings
}

void loop() {
  unsigned long currentMillis = millis();

  checkMQTTConnection();
  client.loop();

  xyzFloat angle = readAndPrintAngles();
  checkPosture(angle);  
  detect_movement();
  delay(1000);
}


void checkPosture(xyzFloat angle) {
  float k = angle.z;

  if (k < 70 && correct_posture) {
    correct_posture = false;
    time_now = millis();
    
  }

  else if (k < 70 && !correct_posture &&
           millis() - time_now > ring_time &&
           millis() - time_now < ring_time + ringing_time) {
    ring_buzzer();
    if (!alert_sent) {
      send_alert("Posture Alert! Incorrect Posture Detected", k);
      alert_sent = true;
      slouchCount++;  
      Serial.print("Slouch Count: ");
      Serial.println(slouchCount);
    }
   
  }

  else if (k < 70 && !correct_posture &&
           millis() - time_now > alert_time) {
    ring_buzzer();  // Assuming this is defined elsewhere
    verifyingPosture = false;
    if (!alert_sent) {
      send_alert("Posture Alert! Continued Incorrect Posture", k);
      alert_sent = true;
    }
  }

  else if (k > 70 && !correct_posture) {
    verify_posture(k);
  }

  else {
    stop_buzzer();
    alert_sent = false;
  }
}



void ring_buzzer(){
  digitalWrite(BUZZER_PIN, HIGH);
}

void stop_buzzer(){
  digitalWrite(BUZZER_PIN, LOW);
}

void verify_posture(float k) {
  if (!verifyingPosture) {
    verifyStartTime = millis();
    verifyingPosture = true;
    stop_buzzer();
  }

  if (k < 70) {
    verifyingPosture = false;  // Verification failed
  } else if (millis() - verifyStartTime >= verifyDuration) {
    correct_posture = true;
    verifyingPosture = false;
    send_alert("Posture Corrected", k);
    alert_sent = false;
    
  }
}


void send_alert(String message, float angle) {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["message"] = message;
  jsonDoc["angle"] = angle;

  char messageBuffer[256];
  serializeJson(jsonDoc, messageBuffer);

  client.publish(topic, messageBuffer);
  Serial.println("MQTT Sent: " + String(messageBuffer));
}

xyzFloat readAndPrintAngles() {
  xyzFloat angle = myMPU9250.getAngles();
  Serial.print("Angle Z = ");
  Serial.println(angle.z);
  return angle;
}

void checkMQTTConnection() {
  if (!client.connected()) {
    connectMQTT();
  }
}

void detect_movement(){
  if (!detection_started){
    detection_started = true;
    detection_time = millis();
  }
  xyzFloat gyr = myMPU9250.getGyrValues();
  float l = gyr.z;
  float m = gyr.y;
  float n = gyr.x;

  Serial.print("Gyro X = ");
  Serial.print(gyr.x, 2);
  Serial.print(" | Y = ");
  Serial.print(gyr.y, 2);
  Serial.print(" | Z = ");
  Serial.println(gyr.z, 2);

  // if (abs(l)>1 && abs(m)>1 && abs(n)>10){
  //   Serial.println("Movement Detected");
  //   detection_started = false;
  //   idle_alert_sent = false;
  // }


  float gyroMagnitude = sqrt(gyr.x * gyr.x + gyr.y * gyr.y + gyr.z * gyr.z);

  if (gyroMagnitude > 15.0 && abs(n)>15) {
      Serial.println("Movement Detected");
      detection_started = false;
      idle_alert_sent = false;
  }


  if (millis()- detection_time> idle_alert_time){
    if (!idle_alert_sent){
      send_alert("Idle for too long", 10);
      idle_alert_sent = true;
      detection_started = false;
    }

  }

}

void connectWithWiFiManager() {
  WiFi.mode(WIFI_STA);  // Set to station mode

  WiFiManager wm;

  // Optional: Uncomment the next line to force reconfiguration each boot
  // wm.resetSettings(); 

  bool res = wm.autoConnect("PostureMonitorAP", "123455678");

  if (!res) {
    Serial.println("❌ Failed to connect");
    // Optional: you could add ESP.restart(); here
  } else {
    Serial.println("✅ Connected to WiFi!");
    espClient.setInsecure();  // Use insecure SSL (skip certificate validation)
  }
}
