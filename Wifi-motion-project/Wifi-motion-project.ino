/*
Goal: capture RSSI data from a target WiFi network using ESP32
Input: nearby WiFi signal strength
Output: timestamped RSSI values in Serial Monitor
Format: time,rssi
Purpose: use signal fluctuations to help detect motion 
*/

#include "WiFi.h"

String targetSSID = "test";

// Calibration settings
const unsigned long CALIBRATION_TIME_MS = 10000;  // 10 seconds

float baselineRSSI = 0;

// sets up WiFi scanning + calibration
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  Serial.println("Starting calibration...");
  Serial.println("Keep environment still.");

  unsigned long startTime = millis();
  long sum = 0;
  int count = 0;

  // --- CALIBRATION LOOP ---
  while (millis() - startTime < CALIBRATION_TIME_MS) {
    int n = WiFi.scanNetworks();

    for (int i = 0; i < n; i++) {
      if (WiFi.SSID(i) == targetSSID) {
        int rssi = WiFi.RSSI(i);
        sum += rssi;
        count++;

        Serial.print("CAL,");
        Serial.println(rssi);
      }
    }

    // delay(200);  // small delay between scans
  }

  // compute baseline
  if (count > 0) {
    baselineRSSI = (float)sum / count;
    Serial.print("Calibration complete. Baseline RSSI = ");
    Serial.println(baselineRSSI);
  } else {
    Serial.println("Calibration failed: target SSID not found.");
  }

  Serial.println("time,raw_rssi,delta");
}

// repeatedly scans nearby WiFi networks
void loop() {
  int n = WiFi.scanNetworks();
  unsigned long currentTime = millis();

  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i) == targetSSID) {
      int rawRSSI = WiFi.RSSI(i);
      float delta = rawRSSI - baselineRSSI;

      Serial.print(currentTime);
      Serial.print(",");
      Serial.print(rawRSSI);
      Serial.print(",");
      Serial.println(delta);
    }
  }
}