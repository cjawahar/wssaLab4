#include <SPI.h>
#include "FXOS8700CQ.h"
#include <FreeRTOS_ARM.h>

FXOS8700CQ sensor;

float magX, magY, magZ;
int Count;
QueueHandle_t xQueue;
SemaphoreHandle_t sem_r, sem_w; // Read/Write semaphores
portBASE_TYPE s1, s2;

//function to read mag values
static void ReadValues(void* arg) {
  
  while(1) {
    xSemaphoreTake(sem_r, portMAX_DELAY);

    sensor.readMagData();
    xSemaphoreGive(sem_w);  //Same process with sem's as prev labs.
  }
}

// Print out mag values -- previously was in loop()
static void PrintMagValues() {

  magX = sensor.magData.x;
  magY = sensor.magData.y;
  magZ = sensor.magData.z;

  SerialUSB.println("Magnetometer X: ");
  SerialUSB.println(magX, 4);
  SerialUSB.println("Magnetometer Y: ");
  SerialUSB.println(magY, 4);
  SerialUSB.println("Magnetometer Z: ");
  SerialUSB.println(magZ, 4);
}

// Collect Data function
static void CollectData(void *arg) {

  while(1) {

    sensor.readMagData();
    magX = sensor.magData.x;
    magY = sensor.magData.y;
    magZ = sensor.magData.z;

    SerialUSB.println("Magnetometer X: ");
    SerialUSB.println(magX, 4);
    SerialUSB.println("Magnetometer Y: ");
    SerialUSB.println(magY, 4);
    SerialUSB.println("Magnetometer Z: ");
    SerialUSB.println(magZ, 4);
  }
}

// Process Data function -- simple call to print out the #
// Explicit in writeup -- could have added to CollectData?
void ProcessData() {

  Count++;
  SerialUSB.println("Processed Data Count:  ");
  SerialUSB.println(Count);
}

void setup() {

  int maxCount = 10000;
  
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
  delay(2000);
}
