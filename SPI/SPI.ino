#include <SPI.h>
#include "FXOS8700CQ.h"

FXOS8700CQ sensor;

void setup() {
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  // Initialize SerialUSB
  while (!SerialUSB);
  SerialUSB.begin(9600);

  // Initialize SPI
  SPI.begin();

  // Initialize sensor
  sensor = FXOS8700CQ();
  sensor.init();

  delay(100);
  sensor.checkWhoAmI();
}

void loop() {
  //Fill code here to read from chip
  SerialUSB.println("Magnetometer Value - X:");
  SerialUSB.println(sensor.magData.x);
  SerialUSB.println("Magnetometer Value - Y:");
  SerialUSB.println(sensor.magData.y);
  SerialUSB.println("Magnetometer Value - Z:");
  SerialUSB.println(sensor.magData.z);

  SerialUSB.println("");
  sensor.checkWhoAmI();
  delay(2000);
}
