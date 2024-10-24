#ifndef I2C_H
#define I2C_H

// I2C communication functions for RTC and EEPROM
int I2C_burstWrite(int slaveADDR, unsigned char memADDR, int byteCount, unsigned char* data);  // Write multiple bytes via I2C
int I2C_burstRead(int slaveADDR, unsigned char memADDR, int byteCount, unsigned char* data);  // Read multiple bytes via I2C

#endif