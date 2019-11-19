#include <SPI.h>
#include "FXOS8700CQ.h"
#include <FreeRTOS_ARM.h>

FXOS8700CQ sensor;

uint32_t magX, magY, magZ;
uint32_t dataCount, loopCount;

SemaphoreHandle_t sem;
TaskHandle_t handleCollec, handleProcess;

// Collect Data function
static void CollectData(void *arg) {

  while(1) {
    SerialUSB.println("Data Collection started.");

    xSemaphoreTake(sem, portMAX_DELAY);
    sensor.resetInterrupt();
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

    vTaskResume(handleProcess);
  }
}

// Process Data function -- simple call to print out the #
// Explicit in writeup -- could have added to CollectData?
void ProcessData(void* arg) {
  
  SerialUSB.println("Processed Data Count:  ");
  dataCount++;
  SerialUSB.println(Count);
}

// Need an Interrupt Service Routine next
void ISR() {
  BaseType_t xHighPriorityTaskAwaken = pdFALSE;
  
  SerialUSB.println("Entered ISR!");

  xSemaphoreGiveFromISR(sem, &xHighPriorityTaskAwaken);
}

void setup() {
  portBASE_TYPE s1, s2;
  
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

  SerialUSB.println("Enabling interrupts/calibration for lab");
  sensor.calibrateMag();
  sensor.beginInterrupt();
  sensor.enableMagInterrupt();

  SerialUSB.println("Beginning FreeRTOS setup");
  sem = xSemaphoreCreateCounting(1, 1);

  SerialUSB.println("Setup FreeRTOS tasks.");
  s1 = xTaskCreate(CollectData, NULL, configMINIMAL_STACK_SIZE, NULL, 1, &handleCollec);
  s2 = xTaskCreate(ProcessData, NULL, configMINIMAL_STACK_SIZE, NULL, 2, &handleProcess);
  
  pinMode(INT_PIN, INPUT); // Interrupt pin
  attachInterrupt(INT_PIN, ISR, FALLING);
  SerialUSB.println("Interrupt pin should be attached!");

  vTaskSuspend(handleProcessData);
  vTaskStartScheduler();
  SerialUSB.println("End of Setup");
}

//// Print out mag values -- previously was in loop()
//static void PrintMagValues() {
//
//  magX = sensor.magData.x;
//  magY = sensor.magData.y;
//  magZ = sensor.magData.z;
//
//  SerialUSB.println("Magnetometer X: ");
//  SerialUSB.println(magX, 4);
//  SerialUSB.println("Magnetometer Y: ");
//  SerialUSB.println(magY, 4);
//  SerialUSB.println("Magnetometer Z: ");
//  SerialUSB.println(magZ, 4);
//}
//
////function to read mag values
//static void ReadValues(void* arg) {
//  
//  while(1) {
//    xSemaphoreTake(sem, portMAX_DELAY);
//
//    sensor.readMagData();
//    xSemaphoreGive(sem_w);  //Same process with sem's as prev labs.
//  }
//}

void loop() {
  
}
