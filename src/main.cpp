#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPL3115A2.h>

// NodeMCU pin names (change if you use a different board)
constexpr uint8_t TRIG_PIN = D5;   // GPIO 14
constexpr uint8_t ECHO_PIN = D6;   // GPIO 12

// I²C: SDA=D2, SCL=D1 on most ESP8266 boards
Adafruit_MPL3115A2 mpl;

/**
 * Return distance in **centimetres**.
 * Returns NaN if the HC‑SR04 times out (>5 m).
 */
float getDistanceCm(float tempC)
{
  const float soundSpeed = 331.3f + 0.606f * tempC;   // m s‑1

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // round‑trip time in µs; 30 000 µs ≈ 5 m
  const unsigned long dur = pulseIn(ECHO_PIN, HIGH, 30000);

  if (dur == 0)           // timeout ⇒ no echo
    return NAN;

  const float dist_m = (dur * 1e-6f) * soundSpeed * 0.5f; // m
  return dist_m * 100.0f;                                 // → cm
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(D2, D1);        // SDA, SCL

  if (!mpl.begin()) {
    Serial.println(F("Could not find MPL3115A2 sensor – check wiring"));
    while (true) delay(10);
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop()
{
  const float tempC      = mpl.getTemperature();
  const float currentCm  = getDistanceCm(tempC);

  bool isCollision      = false;
  bool collisionInbound = false;

  if (!isnan(currentCm)) {
    if (currentCm <= 3.0f) {                        // extremely close
      isCollision = getDistanceCm(tempC) >= 1000.0f; // HC‑SR04 overflow
    }
    else if (currentCm <= 15.0f) {                  // within 15 cm
      collisionInbound = getDistanceCm(tempC) < currentCm;
    }
  }

  Serial.print(F("T = "));
  Serial.print(tempC, 1);
  Serial.print(F(" °C | D = "));
  Serial.print(currentCm, 1);
  Serial.print(F(" cm"));

  if (isCollision)      Serial.print(F("  **COLLISION**"));
  if (collisionInbound) Serial.print(F("  **INBOUND**"));

  Serial.println();
  delay(100);    // avoid spamming the sensor
}

