#include "timer.h"
#include "msp.h"

// Initialize the system clock to 48 MHz using HFXT (external 48 MHz crystal)
void Clock_Init48MHz(void) {
    // Configure Flash wait-state to 1 for both banks 0 & 1
    FLCTL->BANK0_RDCTL = (FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK0_RDCTL_WAIT_1;
    FLCTL->BANK1_RDCTL = (FLCTL->BANK0_RDCTL & ~FLCTL_BANK0_RDCTL_WAIT_MASK) | FLCTL_BANK1_RDCTL_WAIT_1;

    // Configure HFXT to use 48 MHz crystal, source to MCLK & HSMCLK
    PJ->SEL0 |= BIT2 | BIT3;  // Set PJ.2/3 for HFXT function
    PJ->SEL1 &= ~(BIT2 | BIT3);
    CS->KEY = CS_KEY_VAL;  // Unlock CS module for register access
    CS->CTL2 |= CS_CTL2_HFXT_EN | CS_CTL2_HFXTFREQ_6 | CS_CTL2_HFXTDRIVE;
    while (CS->IFG & CS_IFG_HFXTIFG) {  // Wait for HFXT to stabilize
        CS->CLRIFG |= CS_CLRIFG_CLR_HFXTIFG;
    }

    // Set MCLK and HSMCLK to 48 MHz from HFXT, no divider
    CS->CTL1 = (CS->CTL1 & ~(CS_CTL1_SELM_MASK | CS_CTL1_DIVM_MASK | CS_CTL1_SELS_MASK | CS_CTL1_DIVHS_MASK)) |
               CS_CTL1_SELM__HFXTCLK | CS_CTL1_SELS__HFXTCLK;
    CS->KEY = 0;  // Lock CS module from unintended accesses
}

// Initialize the SysTick timer for generating delays
void SysTick_Init(void) {
    SysTick->CTRL = 0;  // Disable SysTick during setup
    SysTick->LOAD = 0x00FFFFFF;  // Maximum reload value
    SysTick->VAL = 0;  // Clear current value
    SysTick->CTRL = 0x00000005;  // Enable SysTick with system clock, no interrupts
}

// Generate a delay in milliseconds using SysTick
void Delay_ms(int delay) {
    SysTick->LOAD = (delay * 48000) - 1;  // Set reload value for 1 millisecond delay
    SysTick->VAL = 0;  // Clear current value
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));  // Wait for the COUNTFLAG to be set
}
12. rotary_encoder.h
c
Copy code
#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

// Function prototypes for handling rotary encoder inputs
void rotaryInit(void);  // Initialize the rotary encoder hardware
void handle_rotary_encoder(void);  // Handle the rotary encoder input and adjust state

#endif