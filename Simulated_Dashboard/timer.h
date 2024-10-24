#ifndef TIMER_H
#define TIMER_H

// Timer initialization and utility functions
void Clock_Init48MHz(void);  // Initialize the system clock to 48 MHz
void SysTick_Init(void);  // Initialize the SysTick timer
void Delay_ms(int delay);  // Create a millisecond delay using SysTick

#endif