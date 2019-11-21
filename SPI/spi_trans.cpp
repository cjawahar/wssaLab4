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
#ifdef SPI_DEBUG
  SerialUSB.println("Write over SPI");
#endif 
  digitalWrite(CS_PIN, LOW); //chip select low active
  delay(1); //give some time for slave to intialize
  SPI.beginTransaction(SPISettings(SPI_HZ, SPI_ORDER, SPI_MODE)); //configure parameters

  SPI.transfer(0x80 | (0x7f & address)); //RW bit and lower 7 bits of address
  SPI.transfer(address); //highest bit of address; remaining  bits are don't cares
  SPI.transfer(tx_data); // transfer the data itself 

  digitalWrite(CS_PIN, HIGH); //release chip select
  SPI.endTransaction();

#ifdef SPI_DEBUG
  SerialUSB.println("Finish write over SPI");
#endif 
}

//------------------------------------------------------------------------------
// spi_read_cmd(): Read from a SPI device. Return the data read from register
//------------------------------------------------------------------------------
uint8_t spi_read_cmd(uint8_t address) {
  // Pg 17 of dataSheet   
  // Read takes 8 bit data from address
  
 #ifdef SPI_DEBUG
  SerialUSB.println("Read over SPI");
#endif 

  uint8_t rx_data;

  digitalWrite(CS_PIN, LOW);
  delay(1);
  SPI.beginTransaction(SPISettings(SPI_HZ, SPI_ORDER, SPI_MODE));
  
  SPI.transfer(0x7f & address);
  SPI.transfer(address);
  rx_data = SPI.transfer(0);

  digitalWrite(CS_PIN, HIGH); //release chip select
  SPI.endTransaction();

#ifdef SPI_DEBUG
  SerialUSB.println("Finish read over SPI");
#endif 

  return rx_data;
}

//*****************************************************************************
