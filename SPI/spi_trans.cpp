// Reading and Writing the SPI bus

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "spi_trans.h"

//******************************************************************************
// Function Definitions
//------------------------------------------------------------------------------
#define GENERIC_SPI_TRANSFER_MODE 1
#define FXOS8700_SPI_TRANSFER_MODE 2
#define SPI_TRANSFER_MODE FXOS8700_SPI_TRANSFER_MODE
//------------------------------------------------------------------------------
// spi_write_cmd(): Transmit to a SPI device
//------------------------------------------------------------------------------
void spi_write_cmd(uint8_t address, uint8_t tx_data) {

  //SerialUSB.println("SPI Write starting here!");

  SPI.beginTransaction(SPISettings(SPI_HZ, SPI_ORDER, SPI_MODE));
#if SPI_TRANSFER_MODE == FXOS8700_SPI_TRANSFER_MODE

  SPI.transfer(0x80 | (0x7f & address));
  SPI.transfer(address);
  SPI.transfer(tx_data);
  
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  //SerialUSB.println("SPI Write finished!");
}

//------------------------------------------------------------------------------
// spi_read_cmd(): Read from a SPI device. Return the data read from register
//------------------------------------------------------------------------------
uint8_t spi_read_cmd(uint8_t address) {
  // Pg 17 of dataSheet   
  // Read takes 8 bit data from address
  
  uint8_t incoming_data;

  //SerialUSB.println("SPI Read starting here!");

  digitalWrite(CS_PIN, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_HZ, SPI_ORDER, SPI_MODE));

  SPI.transfer(0X7F & address);
  SPI.transfer(address);
  incoming_data = SPI.transfer(0);
  
  digitalWrite(CS_PIN, HIGH);
  SPI.endTransaction();

  //SerialUSB.println("SPI Read finished!");

  return incoming_data;
}

//*****************************************************************************
