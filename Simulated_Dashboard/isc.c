#include "i2c.h"
#include "msp.h"

// Perform a burst write of multiple bytes to an I2C device
int I2C_burstWrite(int slaveADDR, unsigned char memADDR, int byteCount, unsigned char* data) {
    __disable_irq();  // Disable interrupts during I2C transaction
    if (byteCount <= 0) {
        return -1;  // Return error if byte count is zero or negative
    }

    EUSCI_B0->I2CSA = slaveADDR;  // Set the I2C slave address
    EUSCI_B0->CTLW0 |= UCTR;  // Set transmitter mode
    EUSCI_B0->CTLW0 |= UCTXSTT;  // Generate START condition and send slave address
    while (EUSCI_B0->CTLW0 & UCTXSTT);  // Wait for START to complete
    EUSCI_B0->TXBUF = memADDR;  // Send memory address to the slave

    // Write all bytes to the I2C device
    for (int i = 0; i < byteCount; i++) {
        while (!(EUSCI_B0->IFG & UCTXIFG));  // Wait for TX buffer to be empty
        EUSCI_B0->TXBUF = data[i];  // Send byte
    }

    while (!(EUSCI_B0->IFG & UCTXIFG));  // Wait for last byte transmission
    EUSCI_B0->CTLW0 |= UCTXSTP;  // Generate STOP condition
    while (EUSCI_B0->CTLW0 & UCTXSTP);  // Wait for STOP to complete
    __enable_irq();  // Enable interrupts

    return 0;  // Return success
}

// Perform a burst read of multiple bytes from an I2C device
int I2C_burstRead(int slaveADDR, unsigned char memADDR, int byteCount, unsigned char* data) {
    __disable_irq();  // Disable interrupts during I2C transaction
    if (byteCount <= 0) {
        return -1;  // Return error if byte count is zero or negative
    }

    EUSCI_B0->I2CSA = slaveADDR;  // Set the I2C slave address
    EUSCI_B0->CTLW0 |= UCTR;  // Set transmitter mode
    EUSCI_B0->CTLW0 |= UCTXSTT;  // Generate START condition and send slave address
    while (EUSCI_B0->CTLW0 & UCTXSTT);  // Wait for START to complete
    EUSCI_B0->TXBUF = memADDR;  // Send memory address to the slave

    while (!(EUSCI_B0->IFG & UCTXIFG));  // Wait for last byte transmission
    EUSCI_B0->CTLW0 &= ~UCTR;  // Set receiver mode
    EUSCI_B0->CTLW0 |= UCTXSTT;  // Generate repeated START condition
    while (EUSCI_B0->CTLW0 & UCTXSTT);  // Wait for START to complete

    // Read all bytes from the I2C device
    for (int i = 0; i < byteCount; i++) {
        if (i == byteCount - 1) {
            EUSCI_B0->CTLW0 |= UCTXSTP;  // Generate STOP after the last byte
        }
        while (!(EUSCI_B0->IFG & UCRXIFG));  // Wait for RX buffer to be full
        data[i] = EUSCI_B0->RXBUF;  // Read received byte
    }

    while (EUSCI_B0->CTLW0 & UCTXSTP);  // Wait for STOP to complete
    __enable_irq();  // Enable interrupts

    return 0;  // Return success
}