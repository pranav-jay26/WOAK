#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_MPL3115A2.h"

#define TRIG_PIN D5
#define ECHO_PIN D6

Adafruit_MPL3115A2 mpl = Adafruit_MPL3115A2();

float getDistance(float temp_C) {
  float soundSpeed = 331.3 + (0.606 + temp_C);

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);

  float distanceMeters = (duration / 1e6) * soundSpeed / 2;

  return distanceMeters * 100;

}

void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);

  if (!mpl.begin()) {
    Serial.println("Could not find MPL3115A2 sensor");
    while(1);
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  float temperature_C = mpl.getTemperature();
  float distance_cm = getDistance(temperature_C);

  Serial.print("Temperature: ");
  Serial.print(temperature_C);
  Serial.print(" °C, Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

}
