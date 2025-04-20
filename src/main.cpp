#include <Arduino.h>
#include <Wire.h>
#include <LittleFS.h>
#include <Adafruit_MPL3115A2.h>
#include <ESP8266Wifi.h>
#include <ESP8266HTTPClient.h>

struct Config { String ssid, pass, url; } cfg;

bool loadDotEnv(const char* path, Config&, out) {
  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS mount failed"));
    return false;
  }
  
  File f = LittleFS.open(path, "r");
  if (!f) { Serial.println(F(" .env not found")); return false; }

  while (f.available()) {
    String line = f.readStringUntil("\n");
    line.trim();
    if (line.startsWith("#") || line.isEmpty()) { continue; }

    int idx = line.indexOf("=");
    if (idx < 0) continue;
    String key = line.substring(0, idx);
    String val = line.substring(idx + 1);
    key.trim(); val.trim();
    
    if (key == "WIFI_SSID") out.ssid = val;
    if (key == "WIFI_PASS") out.pass = val;
    if (key == "API_URL") out.url = val;
  }

  f.close();
  return !(out.ssid.isEmpty() || out.pass.isEmpty() || out.url.isEmpty());
}

constexpr uint8_t TRIG_PIN = D5;
constexpr uint8_t ECHO_PIN = D6;


Adafruit_MPL3115A2 mpl;

float getDistanceCm(float tempC)
{
  const float soundSpeed = 331.3f + 0.606f * tempC;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000);

  if (dur == 0)
    return NAN;

  const float dist_m = (dur * 1e-6f) * soundSpeed * 0.5f;
  return dist_m * 100.0f;
}
void setup() {
  Serial.begin(115200);

  if (!loadDotEnv("/.env", cfg)) {
    Serial.println(F("✗ Couldn’t load config – stopping"));
    while (true) delay(1000);
  }

  Serial.printf("SSID: %s  URL: %s\n", cfg.ssid.c_str(), cfg.url.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.ssid, cfg.pass);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(400);
  }

  Serial.printf("\n✓ Wi‑Fi connected – IP=%s\n", WiFi.localIP().toString().c_str());

  Wire.begin(D2, D1);

  if (!mpl.begin()) {
    Serial.println(F("Could not find MPL3115A2 sensor – check wiring"));
    while (true) delay(10);
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) return;

  const float tempC      = mpl.getTemperature();
  const float currentCm  = getDistanceCm(tempC);

  bool isCollision      = false;
  bool collisionInbound = false;

  if (!isnan(currentCm)) {
    if (currentCm <= 3.0f) {
      isCollision = getDistanceCm(tempC) >= 1000.0f;
    }
    else if (currentCm <= 15.0f) {
      collisionInbound = getDistanceCm(tempC) < currentCm;
    }
  }

  Serial.print(F("T = "));
  Serial.print(tempC, 1);
  Serial.print(F(" °C | D = "));
  Serial.print(currentCm, 1);
  Serial.print(F(" cm"));

  StaticJsonDocument<128> doc;
  doc["device"] = "esp8266-env";
  doc["millis"] = millis();
  doc["celsius"] = tempC;
  doc["dist"] = currentCm;
  String json;
  serializeJson(doc, json);

    
  if (isCollision)      Serial.print(F("  **COLLISION**"));
  if (collisionInbound) Serial.print(F("  **INBOUND**"));

  Serial.println();
  delay(100);

  WiFiClient client;
  HTTPClient  http;

  if (http.begin(client, cfg.url)) {
    http.addHeader("Content-Type", "application/json");
    Serial.printf("POST → %s  ", cfg.url.c_str());
    Serial.printf("HTTP %d\n", http.POST(json));
    http.end();
  }
}

