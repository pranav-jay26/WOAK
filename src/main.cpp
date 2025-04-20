#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPL3115A2.h>


constexpr uint8_t TRIG_PIN = D5;   // GPIO 14
constexpr uint8_t ECHO_PIN = D6;   // GPIO 12


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

void setup()
{
  Serial.begin(115200);
  Wire.begin(D2, D1);

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

  if (isCollision)      Serial.print(F("  **COLLISION**"));
  if (collisionInbound) Serial.print(F("  **INBOUND**"));

  Serial.println();
  delay(100);
}

