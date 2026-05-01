#include "WiFi.h"
#include <HTTPClient.h>

const char* WIFI_SSID = "ORBI56";
const char* WIFI_PASS = "boldvalley437";
const char* PC_URL    = "http://10.0.0.57:5000/motion";
const char* targetBSSID = "0E:36:C9:04:70:75";

const float MOTION_THRESHOLD_DBm = 4.0;
const unsigned long COOLDOWN_MS = 30000;
const float BASELINE_ALPHA = 0.05;  // how fast baseline tracks drift (smaller = slower)

float baselineRSSI = 0;
bool baselineInitialized = false;
unsigned long lastAlertTime = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connect failed - halting");
    while (true) delay(1000);
  }
  Serial.print("\nConnected. IP: ");
  Serial.println(WiFi.localIP());

  // Quick calibration: average a few samples to seed the baseline
  Serial.println("Calibrating baseline (10 s)...");
  long sum = 0;
  int count = 0;
  unsigned long calStart = millis();
  while (millis() - calStart < 10000) {
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      if (WiFi.BSSIDstr(i).equalsIgnoreCase(targetBSSID)) {
        sum += WiFi.RSSI(i);
        count++;
      }
    }
    WiFi.scanDelete();
  }

  if (count > 0) {
    baselineRSSI = (float)sum / count;
    baselineInitialized = true;
    Serial.print("Baseline: ");
    Serial.print(baselineRSSI);
    Serial.println(" dBm");
  } else {
    Serial.println("Calibration failed — target BSSID not seen. Halting.");
    while (true) delay(1000);
  }

  Serial.println("time,raw_rssi,baseline,delta");
}

void sendMotionAlert(float delta, int rssi) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi down, skipping alert");
    return;
  }
  HTTPClient http;
  http.begin(PC_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"delta\":" + String(delta, 2) +
                   ",\"rssi\":" + String(rssi) +
                   ",\"timestamp\":" + String(millis()) + "}";

  int code = http.POST(payload);
  Serial.print("Alert sent, HTTP code: ");
  Serial.println(code);
  http.end();
}

void loop() {
  int n = WiFi.scanNetworks();
  unsigned long currentTime = millis();

  for (int i = 0; i < n; i++) {
    if (WiFi.BSSIDstr(i).equalsIgnoreCase(targetBSSID)) {
      int rawRSSI = WiFi.RSSI(i);
      float delta = rawRSSI - baselineRSSI;
      Serial.println("time,raw_rssi,baseline,delta");
      Serial.print(currentTime);
      Serial.print(",");
      Serial.print(rawRSSI);
      Serial.print(",");
      Serial.print(baselineRSSI, 2);
      Serial.print(",");
      Serial.println(delta);

      if (abs(delta) > MOTION_THRESHOLD_DBm &&
          currentTime - lastAlertTime > COOLDOWN_MS) {
        Serial.println(">>> Motion detected, sending alert");
        sendMotionAlert(delta, rawRSSI);
        lastAlertTime = currentTime;
        // Don't update baseline during a motion event — it would chase the spike
      } else if (abs(delta) < MOTION_THRESHOLD_DBm) {
        // Slowly update baseline only when no motion is detected
        baselineRSSI = (1.0 - BASELINE_ALPHA) * baselineRSSI + BASELINE_ALPHA * rawRSSI;
      }
    }
  }
  WiFi.scanDelete();
}