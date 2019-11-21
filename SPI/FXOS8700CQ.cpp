#include "FXOS8700CQ.h"
#include <math.h>

//******************************************************************************
// Public Function Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// FXOS8700CQ(): Initialize configuration parameters
//------------------------------------------------------------------------------
FXOS8700CQ::FXOS8700CQ() {
		magODR = MODR_100HZ; // Magnetometer data/sampling rate
		magOSR = MOSR_5;     // Choose magnetometer oversample rate

    pinMode(CS_PIN, OUTPUT);        // Select the GPIO Pin 51 as SPI Chip Select
    digitalWrite(CS_PIN, HIGH);     // Set Pin to high (Active Low)
}

//------------------------------------------------------------------------------
// writeReg(): Writes to a register
//------------------------------------------------------------------------------
void FXOS8700CQ::writeReg(uint8_t reg, uint8_t data) {
  spi_write_cmd(reg, data);
}

//------------------------------------------------------------------------------
// readReg(): Reads from a register
//------------------------------------------------------------------------------
uint8_t FXOS8700CQ::readReg(uint8_t reg) {
  return (spi_read_cmd(reg));
}

//------------------------------------------------------------------------------
// readMagData(): Read the magnometer X, Y and Z axisdata
//------------------------------------------------------------------------------
void FXOS8700CQ::readMagData() {
  
  magData.x = readReg(FXOS8700CQ_M_OUT_X_LSB);
  magData.x |= readReg(FXOS8700CQ_M_OUT_X_MSB) << 8;
  magData.x -= calData.avgX;

  magData.y = readReg(FXOS8700CQ_M_OUT_Y_LSB);
  magData.y |= readReg(FXOS8700CQ_M_OUT_Y_MSB) << 8;
  magData.y -= calData.avgY;
  
  magData.z = readReg(FXOS8700CQ_M_OUT_Z_LSB);
  magData.z |= readReg(FXOS8700CQ_M_OUT_Z_MSB) << 8;
  magData.z -= calData.avgZ;
}
//------------------------------------------------------------------------------
// standby(): Put the FXOS8700CQ into standby mode for writing to registers
//------------------------------------------------------------------------------
void FXOS8700CQ::standby() {
  uint8_t register1 = readReg(FXOS8700CQ_CTRL_REG1);
  // place in standby mode.
  writeReg(FXOS8700CQ_CTRL_REG1, (register1 & 0xFE));
}

//------------------------------------------------------------------------------
// active(): Put the FXOS8700CQ into active mode to output data
//------------------------------------------------------------------------------
void FXOS8700CQ::active() {
  uint8_t register1 = readReg(FXOS8700CQ_CTRL_REG1);
  // place in active mode.
  writeReg(FXOS8700CQ_CTRL_REG1, (register1 | 0x1));
}

//------------------------------------------------------------------------------
// init(): Initialize the magnetometer
//         This function will put the magnetometer in standby mode, modify the 
//         registers that put the device in mag-only mode, set the correct data
//         rate (ODR) and oversampling rate (OSR) for the magnetometer and put
//         it back in active mode
//------------------------------------------------------------------------------
void FXOS8700CQ::init() {
    //SPI.beginTransaction (SPISettings (1000000, MSBFIRST, SPI_MODE0));
    standby();
    
    uint8_t first_config, second_config;
    
    first_config = readReg(FXOS8700CQ_CTRL_REG1);
    first_config |= (magODR << 3);
    
    writeReg(FXOS8700CQ_CTRL_REG1, first_config);

    second_config = readReg(FXOS8700CQ_M_CTRL_REG1);
    second_config &= 0xE0;
    second_config |= (magOSR << 2) | 0x1;
    
    writeReg(FXOS8700CQ_M_CTRL_REG1, second_config);
    active();
}

//------------------------------------------------------------------------------
// Interrupt functions for Lab 4 - Enabling Interrupts
//------------------------------------------------------------------------------

void FXOS8700CQ::beginInterrupt() {

  standby();
  resetInterrupt();

  SerialUSB.println("beginInterrupt");

  writeReg(FXOS8700CQ_M_VECM_CFG, 0x70);

  // Update MSB and LSB registers
  writeReg(FXOS8700CQ_M_VECM_INITX_MSB, calData.avgX >> 8);
  writeReg(FXOS8700CQ_M_VECM_INITX_LSB, calData.avgX & 0xFF);
  writeReg(FXOS8700CQ_M_VECM_INITY_MSB, calData.avgY >> 8);
  writeReg(FXOS8700CQ_M_VECM_INITY_LSB, calData.avgY & 0xFF);
  writeReg(FXOS8700CQ_M_VECM_INITZ_MSB, calData.avgZ >> 8);
  writeReg(FXOS8700CQ_M_VECM_INITZ_LSB, calData.avgZ & 0xFF);

  // Next, set interrupt values in magnitude registers.
  writeReg(FXOS8700CQ_M_VECM_CFG, 0x7F);
  writeReg(FXOS8700CQ_M_VECM_THS_MSB, thresholds.Upper);
  //SerialUSB.println("thresholds U: " + thresholds.Upper);
  //SerialUSB.println("thresholds L: " + thresholds.Lower);
  writeReg(FXOS8700CQ_M_VECM_THS_LSB, thresholds.Lower);
  writeReg(FXOS8700CQ_M_VECM_CNT, DEBOUNCE_COUNT); // Debounce set to 25, given.

  active();
}

void FXOS8700CQ::endInterrupt() {
  
  standby();
  
  int8_t register_config = readReg(FXOS8700CQ_M_VECM_CFG);
  writeReg(FXOS8700CQ_M_VECM_CFG, register_config & ~0x02);
  resetInterrupt();
  active();
}

void FXOS8700CQ::resetInterrupt(void) {
  readReg(FXOS8700CQ_M_INT_SRC);
}

void FXOS8700CQ::calc_ISR_Threshold(void) {
  int32_t avg_stdev, avg_Mag;

  avg_Mag = sqrt(pow(calData.avgX, 2) + pow(calData.avgY, 2) + pow(calData.avgZ, 2));
  avg_stdev = sqrt(pow(calData.stdX, 2) + pow(calData.stdY, 2) + pow(calData.stdZ, 2));

  int16_t threshold = (int16_t) 4*avg_stdev;

  thresholds.Lower = threshold & 0xFF;
  thresholds.Upper = threshold >> 8;
}


// Need to call readMagData() within this function and then calculate a running average

void FXOS8700CQ::calibrateMag() {
  //Using 32 bits here -- readMagData is 16, need to cast.
  uint32_t x=0, y=0, z=0;
  uint32_t avgX, avgY, avgZ;
  uint32_t xSquare, ySquare, zSquare;

  SerialUSB.println("Magnetometer is being calibrated...");

  int i = 0, calNum = 20;
  
  //collect data for some time, calculate the average
  while(i++ < calNum) {
    readMagData(); // Should be able to call magData.x after this
    x += (int32_t) magData.x;
    xSquare += magData.x * magData.x;
     
    y += (int32_t) magData.y;
    ySquare += magData.y * magData.y;
     
    z += (int32_t) magData.z;
    zSquare += magData.z * magData.z;
    delay(10);
  }

  //Average calculations
  avgX = (x / i);
  calData.avgX = (int16_t) avgX; 
  calData.stdX = (int16_t) sqrt(xSquare/i - avgX*avgX);

  avgY = (y / i);
  calData.avgY = (int16_t) avgY;
  calData.stdY = (int16_t) sqrt(ySquare/i - avgY*avgY);

  avgZ = (z / i);
  calData.avgZ = (int16_t) avgZ;
  calData.stdZ = (int16_t) sqrt(zSquare/i - avgZ*avgZ);

  SerialUSB.println("calData");
  SerialUSB.println(calData.avgX);
  SerialUSB.println(calData.stdX);
  SerialUSB.println(calData.avgY);
  SerialUSB.println(calData.stdY);
  SerialUSB.println(calData.avgZ);
  SerialUSB.println(calData.stdZ);

  // Now that calData is populated -- actually calculate the ISR threshold
  calc_ISR_Threshold();
  
  SerialUSB.println("Magnetometer calibration complete!");
}
//------------------------------------------------------------------------------
// checkWhoAmI(): Check the whoAmI register
//------------------------------------------------------------------------------
#define FXOS8700CQ_WHO_AM_I_VAL 0xC7
void FXOS8700CQ::checkWhoAmI(void) {
  uint8_t whoAmI = readReg(FXOS8700CQ_WHO_AM_I);

  if (whoAmI == FXOS8700CQ_WHO_AM_I_VAL) {
    SerialUSB.print("Who_Am_I check success!\n");
  }
  else {
    SerialUSB.print("Who_Am_I check failed!\n");
    SerialUSB.print(whoAmI, HEX);
  }
}

//*****************************************************************************
