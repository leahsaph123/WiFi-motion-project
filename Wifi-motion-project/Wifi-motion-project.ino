#include "WiFi.h"
#include <HTTPClient.h>

const char* WIFI_SSID = "ORBI56"; //YOUR NETWORK HERE
const char* WIFI_PASS = "boldvalley437"; //YOUR WIFI PASSWORD HERE
const char* PC_URL    = "http://10.0.0.57:5000/motion";  // YOUR PC's LAN IP HERE

const char* targetBSSID = "0E:36:C9:04:70:75";  // CHOSEN MAC ADDRESS FOR MESH NETWORKS
const float MOTION_THRESHOLD_DB = 4.0;          // tune this
const unsigned long COOLDOWN_MS = 30000;        // don't spam — 30 s between alerts

float baselineRSSI = 0;
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

  // ... your existing calibration loop here ...
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

      Serial.print(currentTime);
      Serial.print(",");
      Serial.print(rawRSSI);
      Serial.print(",");
      Serial.println(delta);

      if (abs(delta) > MOTION_THRESHOLD_DB &&
          currentTime - lastAlertTime > COOLDOWN_MS) {
        Serial.println(">>> Motion detected, sending alert");
        sendMotionAlert(delta, rawRSSI);
        lastAlertTime = currentTime;
      }
    }
  }
  WiFi.scanDelete();
}