#include "spi.h"
#include "msp.h"

// Send a single byte of data via SPI and return the received byte
uint8_t sendCharSPI(uint8_t data) {
    __disable_irq();  // Disable interrupts during SPI transmission
    P6->OUT &= ~BIT2;  // Drive CS pin LOW for SPI device
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG));  // Wait for transmit buffer to be empty
    EUSCI_B1->TXBUF = data;  // Transmit data
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_RXIFG));  // Wait for reception of data
    P6->OUT |= BIT2;  // Drive CS pin HIGH after transmission
    __enable_irq();  // Enable interrupts
    return EUSCI_B1->RXBUF;  // Return the received byte (dummy data)
}

// Write data to a specific address of the 7-segment display over SPI
uint8_t writeData7Seg(uint8_t addr, uint8_t data) {
    __disable_irq();  // Disable interrupts during SPI transmission
    addr ^= 0xFF;  // Invert the address (depends on circuit configuration)
    data ^= 0xFF;  // Invert the data (depends on circuit configuration)
    P5->OUT &= ~BIT5;  // Drive CS pin LOW for 7-segment display
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG));  // Wait for transmit buffer to be empty
    EUSCI_B1->TXBUF = addr;  // Send address byte
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_TXIFG));  // Wait for transmit buffer to be empty
    EUSCI_B1->TXBUF = data;  // Send data byte
    while (!(EUSCI_B1->IFG & EUSCI_B_IFG_RXIFG));  // Wait for reception of data
    P5->OUT |= BIT5;  // Drive CS pin HIGH after transmission
    __enable_irq();  // Enable interrupts
    return EUSCI_B1->RXBUF;  // Return the received byte (dummy data)
}

// Print a number to the 7-segment display using SPI
void SevSegPrint(uint32_t num) {
    int digit;
    int position = 8;  // Start from the last position

    // Clear the display first
    for (int i = 1; i <= 8; i++) {
        writeData7Seg(i, 0x0F);  // Turn off each digit on the display
    }

    // Extract each digit from the number and send it to the display
    while (num > 0 && position > 0) {
        digit = num % 10;  // Extract the last digit
        writeData7Seg(position, digit);  // Write digit to the display
        num /= 10;  // Remove the last digit
        position--;  // Move to the next display position
    }
}