#include <psp_api.h>
#include <stdio.h>
#include <stdlib.h>

#define GPIO_SWs                0x80001400
#define GPIO_LEDs               0x80001404
#define GPIO_INOUT              0x80001408
#define GPIO_BTNs               0x80001800

#define SEGMENTS_DIGIT0         0x80001038
#define SEGMENTS_DIGIT1         0x80001039
#define SEGMENTS_DIGIT2         0x8000103A
#define SEGMENTS_DIGIT3         0x8000103B
#define SEGMENTS_DIGIT4         0x8000103C
#define SEGMENTS_DIGIT5         0x8000103D
#define SEGMENTS_DIGIT6         0x8000103E
#define SEGMENTS_DIGIT7         0x8000103F

#define SEGMENT_S 0x24 // S:  0100100
#define SEGMENT_T 0x70 // t:  1110000
#define SEGMENT_A 0x08 // A:  0001000
#define SEGMENT_R 0x7A // r:  1111010

#define SEGMENT_0 0x01 // 0:  0000001
#define SEGMENT_1 0x79 // 1:  1111001
#define SEGMENT_2 0x12 // 2:  0010010
#define SEGMENT_3 0x06 // 3:  0000110
#define SEGMENT_4 0x4C // 4:  0110011
#define SEGMENT_5 0x24 // 5:  0100100
#define SEGMENT_6 0x60 // 6:  1100000
#define SEGMENT_7 0x0F // 7:  0001111
#define SEGMENT_8 0x00 // 8:  0000000
#define SEGMENT_9 0x0C // 9:  0001100

#define SEGMENT_H 0x48 // H:  1001000
#define SEGMENT_L 0x71 // L:  1110001

#define SEGMENT_U 0x63 // U:  1100011
#define SEGMENT_E 0x30 // E:  0110000

#define SEGMENT_OFF 0x7F  // off:  1111111

#define RPTC_CNTR               0x80001200
#define RPTC_HRC                0x80001204
#define RPTC_LRC                0x80001208
#define RPTC_CTRL               0x8000120C

#define TIMER2_CNTR             0x80001240
#define TIMER2_HRC              0x80001244
#define TIMER2_LRC              0x80001248
#define TIMER2_CTRL             0x8000124C

#define TIMER3_CNTR             0x80001280
#define TIMER3_HRC              0x80001284
#define TIMER3_LRC              0x80001288
#define TIMER3_CTRL             0x8000128C

#define TIMER4_CNTR             0x800012C0
#define TIMER4_HRC              0x800012C4
#define TIMER4_LRC              0x800012C8
#define TIMER4_CTRL             0x800012CC

#define READ_GPIO(dir)          (*(volatile unsigned *)dir)
#define WRITE_GPIO(dir, value)  { (*(volatile unsigned *) dir) = (value); }

#define DELAY                   1000000
#define DEBOUNCE_DELAY          50000

volatile int start_flag = 0;

void delay(int count) {
    volatile int i;
    for (i = 0; i < count; i++);
}

// Function to display a number on the 7-segment display
void display_number_on_7seg(unsigned int num) {
    unsigned int digits[8] = {SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF, SEGMENT_OFF};
    int i = 0;

    // Extract each digit of the number
    do {
        switch (num % 10) {
            case 0: digits[i] = SEGMENT_0; break;
            case 1: digits[i] = SEGMENT_1; break;
            case 2: digits[i] = SEGMENT_2; break;
            case 3: digits[i] = SEGMENT_3; break;
            case 4: digits[i] = SEGMENT_4; break;
            case 5: digits[i] = SEGMENT_5; break;
            case 6: digits[i] = SEGMENT_6; break;
            case 7: digits[i] = SEGMENT_7; break;
            case 8: digits[i] = SEGMENT_8; break;
            case 9: digits[i] = SEGMENT_9; break;
        }
        num /= 10;
        i++;
    } while (num > 0 && i < 8);

    // Write digits to 7-segment display
    WRITE_GPIO(SEGMENTS_DIGIT0, digits[0]);
    WRITE_GPIO(SEGMENTS_DIGIT1, digits[1]);
    WRITE_GPIO(SEGMENTS_DIGIT2, digits[2]);
    WRITE_GPIO(SEGMENTS_DIGIT3, digits[3]);
    WRITE_GPIO(SEGMENTS_DIGIT4, digits[4]);
    WRITE_GPIO(SEGMENTS_DIGIT5, digits[5]);
    WRITE_GPIO(SEGMENTS_DIGIT6, digits[6]);
    WRITE_GPIO(SEGMENTS_DIGIT7, digits[7]);
}

// Function to display a single digit on a specific 7-segment display
void display_single_digit(int digit, int position) {
    unsigned int segment_value;

    switch (digit) {
        case 0: segment_value = SEGMENT_0; break;
        case 1: segment_value = SEGMENT_1; break;
        case 2: segment_value = SEGMENT_2; break;
        case 3: segment_value = SEGMENT_3; break;
        case 4: segment_value = SEGMENT_4; break;
        case 5: segment_value = SEGMENT_5; break;
        case 6: segment_value = SEGMENT_6; break;
        case 7: segment_value = SEGMENT_7; break;
        case 8: segment_value = SEGMENT_8; break;
        case 9: segment_value = SEGMENT_9; break;
        default: segment_value = SEGMENT_OFF; break;
    }

    WRITE_GPIO(position, segment_value);
}

// Function to flash LEDs
void flash_leds(int times, int delay_duration) {
    for (int i = 0; i < times; i++) {
        WRITE_GPIO(GPIO_LEDs, 0xFFFF); // Turn on all LEDs
        delay(delay_duration);
        WRITE_GPIO(GPIO_LEDs, 0x0000); // Turn off all LEDs
        delay(delay_duration);
    }
}

// Function to flash red RGB LED
void flash_red_rgb(int times, int delay_duration) {
    for (int i = 0; i < times; i++) {
        WRITE_GPIO(TIMER4_HRC, 31); // Turn on red RGB LED (set PWM high register to max)
        delay(delay_duration);
        WRITE_GPIO(TIMER4_HRC, 0); // Turn off red RGB LED (set PWM high register to 0)
        delay(delay_duration);
    }
}

// Function to reset the game
void reset_game(unsigned int *random_number, int *attempt_count) {
    // Write "START" to 7-segment display
    start_flag = 0;
    WRITE_GPIO(SEGMENTS_DIGIT0, SEGMENT_OFF); // off
    WRITE_GPIO(SEGMENTS_DIGIT1, SEGMENT_OFF); // off
    WRITE_GPIO(SEGMENTS_DIGIT2, SEGMENT_OFF); // off
    WRITE_GPIO(SEGMENTS_DIGIT3, SEGMENT_T);   // t
    WRITE_GPIO(SEGMENTS_DIGIT4, SEGMENT_R);   // r
    WRITE_GPIO(SEGMENTS_DIGIT5, SEGMENT_A);   // A
    WRITE_GPIO(SEGMENTS_DIGIT6, SEGMENT_T);   // t
    WRITE_GPIO(SEGMENTS_DIGIT7, SEGMENT_S);   // S

    // Generate a new random number between 0 and 99
    *random_number = READ_GPIO(RPTC_CNTR) % 100;  // Ensure the random number is between 0 and 99
    //display_number_on_7seg(*random_number);

    // Initialize attempt count
    *attempt_count = 0;
}

int main(void)
{
    int En_Value = 0xFFFF, btns_value, switches_value, random_number, attempt_count = 0;
    int previous_btns_value = 0; // Track previous button state
    

    WRITE_GPIO(RPTC_LRC, 99); 
    WRITE_GPIO(RPTC_CTRL, 0x01); // enable coutner for RPTC timer. 

    // Enable GPIO
    WRITE_GPIO(GPIO_INOUT, En_Value);

    // Enable RPTC as an incremental counter
    WRITE_GPIO(RPTC_CTRL, 0x01); // Enable counter in incremental mode

    // Enable PWM timers
    WRITE_GPIO(TIMER4_CTRL, 0b1001); // Enable PWM for TIMER4

    // Set lower register to 31 (highest 5 bit number)
    WRITE_GPIO(TIMER4_LRC, 31);

    // Initialize the game
    reset_game(&random_number, &attempt_count);

    while (1) {
        // Read value on switches
        switches_value = READ_GPIO(GPIO_SWs);   // read value on switches
        switches_value = switches_value >> 16;  // shift into lower 16 bits

        // Limit value to 99
        if (switches_value > 99) {
            switches_value = 99;
        }

        // Read value on push buttons
        btns_value = READ_GPIO(GPIO_BTNs);

        // Check if any button is pressed
        if ((btns_value & 0x1F) && !(previous_btns_value & 0x1F)) {
            start_flag = 1;
            // Debounce delay
            delay(DEBOUNCE_DELAY);

            // Check if any button is still pressed
            if (READ_GPIO(GPIO_BTNs) & 0x1F) {
                attempt_count++;
                if (attempt_count > 7) {
                    attempt_count = 0; // Reset count if it exceeds 7
                }

                // Check the entered number
                WRITE_GPIO(GPIO_LEDs, switches_value);  // Display switches value on LEDs
                display_number_on_7seg(switches_value); // Display decimal equivalent on 7-segment display

                // Display "H" or "L" based on the comparison
                if (switches_value > random_number) {
                    WRITE_GPIO(SEGMENTS_DIGIT5, SEGMENT_H); // Display "H" for higher
                    WRITE_GPIO(SEGMENTS_DIGIT6, SEGMENT_OFF); // off
                } else if (switches_value < random_number) {
                    WRITE_GPIO(SEGMENTS_DIGIT5, SEGMENT_L); // Display "L" for lower
                    WRITE_GPIO(SEGMENTS_DIGIT6, SEGMENT_OFF); // off
                } else {
                    WRITE_GPIO(SEGMENTS_DIGIT4, SEGMENT_E); // Display "E" for equal
                    WRITE_GPIO(SEGMENTS_DIGIT5, SEGMENT_U); // Display "U" for equal
                    WRITE_GPIO(SEGMENTS_DIGIT6, SEGMENT_R); // Display "R" for equal
                    WRITE_GPIO(SEGMENTS_DIGIT7, SEGMENT_T); // Display "T" for equal
                    
                    // Flash LEDs 10 times
                    flash_leds(10, DELAY);

                    // Wait for user to press any button to restart
                    while (!(READ_GPIO(GPIO_BTNs) & 0x1F)) {
                        delay(DELAY); 
                    }
                    reset_game(&random_number, &attempt_count);
                }

                if ((switches_value != random_number)&&(start_flag == 1)) {
                    display_single_digit(attempt_count, SEGMENTS_DIGIT7);
                }

                // Flash red RGB LED if attempts are 7 or more
                if (attempt_count >= 7) {
                    flash_red_rgb(10, DELAY);
                    reset_game(&random_number, &attempt_count);
                }
            }
        }

        // Update previous button state
        previous_btns_value = btns_value;

        delay(DELAY);
    }

    return 0;
}
