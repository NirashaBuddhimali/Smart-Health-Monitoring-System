// === Includes ===
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "MAX30105.h"                 // Heart rate & SpO2 sensor
#include "spo2_algorithm.h"          // SpO2 calculation algorithm
#include <math.h>

// === Sensor and Pins ===
MAX30105 sensor;
#define LED_PIN   2                  // LED indicator for status
#define VIB_PIN   25                 // Pin connected to vibration motor
#define TEMP_PIN  34                 // Analog pin for thermistor

// === Wi-Fi Setup ===
const char* ssid = "Kavi";           // Wi-Fi SSID
const char* password = "kavindu15763"; // Wi-Fi Password
WebServer server(80);               // HTTP server on port 80

// === Thresholds for triggering alerts ===
#define HR_THRESHOLD      110       // BPM
#define SPO2_THRESHOLD    94        // Percentage
#define TEMP_THRESHOLD    37.5      // Celsius

// === Thermistor Configuration ===
const float SERIES_RESISTOR = 10000.0;
const float NOMINAL_RESISTANCE = 10000.0;
const float NOMINAL_TEMPERATURE = 25.0;
const float BETA = 3950.0;          // B-constant from thermistor datasheet

// === Sensor Data Buffers ===
#define SAMPLE_COUNT 50             // Number of samples per reading cycle
uint32_t irBuffer[SAMPLE_COUNT];
uint32_t redBuffer[SAMPLE_COUNT];

// === Measured Values ===
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;
float tempC = 0;

// === Alert Flags ===
bool isHRBad = false;
bool isSpO2Bad = false;
bool isTempBad = false;
bool alertTriggered = false;
unsigned long alertStartTime = 0;

// === Latest Sensor Reading for Web ===
String latestJSON = "{\"heartRate\":0,\"spo2\":0,\"temp\":0}";

// === Timing ===
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // 2 seconds

// === Function to Trigger Vibration Alerts ===
void vibratePattern(int pattern) {
  switch (pattern) {
    case 1: digitalWrite(VIB_PIN, HIGH); delay(500); digitalWrite(VIB_PIN, LOW); break;
    case 2:
      for (int i = 0; i < 2; i++) {
        digitalWrite(VIB_PIN, HIGH); delay(150);
        digitalWrite(VIB_PIN, LOW); delay(150);
      }
      break;
    case 3:
      digitalWrite(VIB_PIN, HIGH); delay(200);
      digitalWrite(VIB_PIN, LOW); delay(100);
      digitalWrite(VIB_PIN, HIGH); delay(300);
      digitalWrite(VIB_PIN, LOW);
      break;
  }
}

// === HTML Dashboard Response ===
String htmlPage() {
  String bgColor = (isHRBad || isSpO2Bad || isTempBad) ? "#ff4444" : "#1c1c1c";
  String textColor = (isHRBad || isSpO2Bad || isTempBad) ? "white" : "#00ffcc";
  String html = "<!DOCTYPE html><html><head>"
    "<title>Health Monitor</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
    "<style>"
    "body { background:" + bgColor + "; color:" + textColor + "; font-family: sans-serif; text-align:center; padding:20px; transition:0.3s; }"
    "h1 { font-size: 28px; margin-bottom: 20px; }"
    "p { font-size: 20px; margin: 10px 0; }"
    "button { padding: 12px 24px; margin: 8px; font-size: 16px; background: #222; color: #00ffcc; border: none; border-radius: 8px; cursor:pointer; transition:0.2s; }"
    "button:hover { background: #00ffcc; color: #000; }"
    "canvas { max-width: 100%; height: 200px; }"
    "</style></head><body>"
    "<h1>Smart Health Monitor</h1>"
    "<p id='hr'>Heart Rate: -- bpm</p>"
    "<p id='spo2'>SpO2: -- %</p>"
    "<p id='temp'>Temperature: -- °C</p>"
    "<button onclick=\"sendVibe(1)\">Take Rest</button>"
    "<button onclick=\"sendVibe(2)\">Drink Water</button>"
    "<button onclick=\"sendVibe(3)\">Report to Office</button>"
    "<canvas id='chart'></canvas>"
    "<script>"
    "let hrData = [], labels = [], spo2Data = [], tempData = [];"
    "let chart = new Chart(document.getElementById('chart').getContext('2d'), {type: 'line', data: {labels: labels, datasets: ["
    "{ label: 'Heart Rate', data: hrData, borderColor: '#00ffcc', fill: false },"
    "{ label: 'SpO2', data: spo2Data, borderColor: '#00cc66', fill: false },"
    "{ label: 'Temp', data: tempData, borderColor: '#ffcc00', fill: false }]},"
    "options: {scales: {x: { display: false }, y: { beginAtZero: false }}, animation: false}});"
    "async function fetchData() {"
    "const res = await fetch('/data');"
    "const json = await res.json();"
    "document.getElementById('hr').innerText = 'Heart Rate: ' + json.heartRate + ' bpm';"
    "document.getElementById('spo2').innerText = 'SpO2: ' + json.spo2 + ' %';"
    "document.getElementById('temp').innerText = 'Temperature: ' + json.temp + ' °C';"
    "if (labels.length > 30) { labels.shift(); hrData.shift(); spo2Data.shift(); tempData.shift(); }"
    "let ts = new Date().toLocaleTimeString();"
    "labels.push(ts); hrData.push(json.heartRate); spo2Data.push(json.spo2); tempData.push(json.temp);"
    "chart.update();"
    "}"
    "function sendVibe(pat) { fetch('/vibrate?pattern=' + pat); }"
    "setInterval(fetchData, 2000);"
    "</script></body></html>";
  return html;
}

// === HTTP Request Handlers ===
void handleRoot() { server.send(200, "text/html", htmlPage()); }
void handleVibration() {
  if (server.hasArg("pattern")) vibratePattern(server.arg("pattern").toInt());
  server.send(200, "text/plain", "OK");
}
void handleData() { server.send(200, "application/json", latestJSON); }

// === Setup Function ===
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(VIB_PIN, OUTPUT);
  digitalWrite(VIB_PIN, LOW);
  Wire.begin(21, 22);               // I2C pins

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());

  // Web server routes
  server.on("/", handleRoot);
  server.on("/vibrate", handleVibration);
  server.on("/data", handleData);
  server.begin();

  // MAX30105 sensor initialization
  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found!");
    while (1);
  }
  sensor.setup();
  sensor.setPulseAmplitudeRed(0x3F);
  sensor.setPulseAmplitudeIR(0x7F);
  sensor.setLEDMode(3);
  sensor.setFIFOAverage(8);
  sensor.setSampleRate(100);
  sensor.setPulseWidth(411);
  sensor.setADCRange(4096);
  Serial.println("Sensor initialized");
}

// === Main Loop ===
void loop() {
  server.handleClient();
  unsigned long now = millis();

  // Read data every 2 seconds
  if (now - lastReadTime >= readInterval && sensor.getIR() > 15000) {
    lastReadTime = now;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Reading vitals...");

    // Fill buffers
    for (int i = 0; i < SAMPLE_COUNT; i++) {
      while (!sensor.available()) sensor.check();
      redBuffer[i] = sensor.getRed();
      irBuffer[i] = sensor.getIR();
      sensor.nextSample();
    }

    digitalWrite(LED_PIN, LOW);
    Serial.println("Calculating results...");

    // Calculate HR and SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, SAMPLE_COUNT, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

    // Read temp
    tempC = readTemperature();
    isHRBad   = validHeartRate && heartRate > HR_THRESHOLD;
    isSpO2Bad = validSPO2     && spo2 < SPO2_THRESHOLD;
    isTempBad = tempC > TEMP_THRESHOLD;

    // Alert handling
    if (isHRBad || isSpO2Bad || isTempBad) {
      if (!alertTriggered) {
        alertTriggered = true;
        alertStartTime = now;
        Serial.println("Abnormal detected. Waiting for confirmation...");
      } else if (now - alertStartTime >= 5000) {
        Serial.println("🚨 Triggering alert!");
        alertFlash(); alertVibrate();
        alertTriggered = false;
      }
    } else {
      alertTriggered = false;
    }

    // Update JSON for web
    latestJSON = "{";
    latestJSON += "\"heartRate\":" + String(heartRate / 2) + ",";
    latestJSON += "\"spo2\":" + String(spo2) + ",";
    latestJSON += "\"temp\":" + String(tempC, 1) + "}";
  }
}

// === Read Temperature from Thermistor ===
float readTemperature() {
  int adc = analogRead(TEMP_PIN);
  float voltage = adc * 3.3 / 4095.0;
  float resistance = SERIES_RESISTOR * (3.3 / voltage - 1);
  float steinhart = log(resistance / NOMINAL_RESISTANCE) / BETA + (1.0 / (NOMINAL_TEMPERATURE + 273.15));
  return (1.0 / steinhart) - 273.15;
}

// === LED Alert ===
void alertFlash() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(LED_PIN, HIGH); delay(150);
    digitalWrite(LED_PIN, LOW); delay(150);
  }
}

// === Vibration Motor Alert ===
void alertVibrate() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(VIB_PIN, HIGH); delay(200);
    digitalWrite(VIB_PIN, LOW); delay(100);
    digitalWrite(VIB_PIN, HIGH); delay(300);
    digitalWrite(VIB_PIN, LOW); delay(200);
  }
}
