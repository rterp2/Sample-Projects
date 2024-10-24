#include "msp.h"
#include "display.h"
#include "sensors.h"
#include "spi.h"
#include "i2c.h"
#include "timer.h"
#include "rotary.h"


// Global structure to hold all the dashboard data
DashboardData dashboardData;

int main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;  // Stop watchdog timer

    __disable_irq();  // Disable global interrupts
    Clock_Init48MHz();  // Initialize system clock to 48 MHz
    initialize_peripherals();  // Initialize ADC, I2C, SPI, timers, display, and rotary encoder
    SysTick_Init();  // Initialize SysTick timer for delay
    ST7735_InitR(INITR_REDTAB);  // Initialize the LCD display
    __enable_irq();  // Enable global interrupts

    while (1) {
        // Main loop to handle sensor readings, display updates, and rotary encoder inputs

        if (dashboardData.masterTimer) {
            dashboardData.masterTimer = 0;
            update_speed_and_tachometer(&dashboardData);  // Update speed and RPM gauges
            update_odometer(&dashboardData);  // Update odometer
            update_time_and_date(&dashboardData);  // Pull time and date from RTC
        }

        if (dashboardData.ADCFlag) {
            dashboardData.ADCFlag = 0;
            handle_adc_readings(&dashboardData);  // Handle ADC sensor readings (temperature, water level)
        }

        handle_rotary_encoder();  // Process rotary encoder inputs
        check_blinkers();  // Check and process blinker signals
        update_display(&dashboardData);  // Update the display with latest values
    }
}