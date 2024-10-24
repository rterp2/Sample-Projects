#ifndef SPI_H
#define SPI_H

// SPI communication functions for sending and receiving data
uint8_t sendCharSPI(uint8_t data);  // Send a character via SPI and return the received data
uint8_t writeData7Seg(uint8_t addr, uint8_t data);  // Write data to 7-segment display via SPI
void SevSegPrint(uint32_t num);  // Print a number to the 7-segment display

#endif