#include "rotary_encoder.h"
#include "msp.h"

// Initialize the rotary encoder inputs and buttons
void rotaryInit(void) {
    // Configure rotary encoder pins (S1, S2) and button pin as input
    P3->SEL0 &= ~(BIT3 | BIT2);  // Set P3.2 (S1) and P3.3 (S2) as GPIO
    P3->DIR &= ~(BIT3 | BIT2);  // Set P3.2 and P3.3 as inputs

    P3->REN |= (BIT3 | BIT2);  // Enable pull-up/down resistors
    P3->OUT |= (BIT3 | BIT2);  // Set pull-up resistors

    // Configure the encoder button pin as input with pull-up
    P3->SEL0 &= ~BIT7;
    P3->DIR &= ~BIT7;
    P3->REN |= BIT7;  // Enable pull-up resistor
    P3->OUT |= BIT7;

    // Enable interrupts for the rotary encoder signals
    P3->IES &= ~BIT3;  // Interrupt on low-to-high transition for S1
    P3->IE |= BIT3;  // Enable interrupt on S1
    P3->IFG &= ~BIT3;  // Clear interrupt flag for S1

    P3->IES |= BIT7;  // Interrupt on high-to-low transition for the button
    P3->IE |= BIT7;  // Enable interrupt on the button
    P3->IFG &= ~BIT7;  // Clear interrupt flag for the button
}

// Handle the rotary encoder input, updating states and flags
void handle_rotary_encoder(void) {
    static int forwardSpin = 0, reverseSpin = 0;
    static int lastStateA = 0, lastStateB = 0;

    int aState = (P3->IN & BIT3) >> 3;  // Read the current state of S1
    int bState = (P3->IN & BIT2) >> 2;  // Read the current state of S2

    if (aState != lastStateA) {  // If there was a change on S1
        if (aState != bState) {  // Clockwise rotation detected
            forwardSpin = 1;
        } else {  // Counter-clockwise rotation detected
            reverseSpin = 1;
        }
    }

    // Handle forward or reverse rotation
    if (forwardSpin) {
        forwardSpin = 0;
        // Code to handle forward rotation
    }
    if (reverseSpin) {
        reverseSpin = 0;
        // Code to handle reverse rotation
    }

    lastStateA = aState;  // Store the last state of S1
    lastStateB = bState;  // Store the last state of S2
}
