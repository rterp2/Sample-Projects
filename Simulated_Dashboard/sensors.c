#include "sensors.h"
#include "spi.h"
#include "i2c.h"
#include "timer.h"
#include "rotary_encoder.h"

// Utility function to clamp a value between a min and max range
static float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Initialize the ADC (Analog-to-Digital Converter) for various sensors
void initialize_ADC(void) {
    ADCPORT->SEL0 |= ADCPIN | WATERPIN | TEMPPIN;
    ADCPORT->SEL1 |= ADCPIN | WATERPIN | TEMPPIN;
    ADC14->CTL0 &= ~ADC14_CTL0_ENC;
    ADC14->CTL0 |= ADC14_CTL0_CONSEQ_1 | ADC14_CTL0_SHP | ADC14_CTL0_SSEL__SMCLK | ADC14_CTL0_ON;
    ADC14->CTL1 |= ADC14_CTL1_RES__14BIT;
    ADC14->MCTL[0] |= ADC14_MCTLN_INCH_8;
    ADC14->MCTL[1] |= ADC14_MCTLN_INCH_7;
    ADC14->MCTL[2] |= ADC14_MCTLN_INCH_6 | ADC14_MCTLN_EOS;
    ADC14->IER0 |= ADC14_IER0_IE0;
    ADC14->CTL0 |= ADC14_CTL0_ENC;
}

// Initialize the I2C communication (used for RTC and EEPROM)
void initialize_I2C(void) {
    P1->SEL0 |= (BIT6 | BIT7);  // Set P1.6 (SDA) and P1.7 (SCL) as I2C pins
    EUSCI_B0->CTLW0 |= UCSWRST;  // Hold eUSCI in reset state
    EUSCI_B0->CTLW0 |= UCMODE_3 | UCMST | UCSSEL_2 | UCSYNC;
    EUSCI_B0->BRW = 30;  // Set baud rate for 400kHz I2C communication
    EUSCI_B0->CTLW0 &= ~UCSWRST;  // Release reset
}

// Initialize SPI communication for 7-segment display and other peripherals
void initialize_SPI(void) {
    EUSCI_B1->CTLW0 |= UCSWRST;  // Hold eUSCI in reset state
    EUSCI_B1->CTLW0 |= UCCKPH | UCCKPL | UCMSB | UCMST | UCSYNC | UCSSEL_2;
    EUSCI_B1->BRW = 9;  // Set baud rate for SPI communication
    EUSCI_B1->CTLW0 &= ~UCSWRST;  // Release reset
}

// Initialize timers for timing and PWM purposes
void initialize_TIMERS(void) {
    TIMER_A0->CTL |= TASSEL_1 | MC_2 | TACLR | ID_0;  // Initialize timer A0 for capturing
    TIMER32_1->LOAD = 24000000;  // Timer for periodic events
    TIMER32_1->CONTROL |= TIMER32_CONTROL_ENABLE;  // Enable Timer32
}

// Enable interrupts for all necessary peripherals
void enable_interrupts(void) {
    NVIC_EnableIRQ(ADC14_IRQn);  // Enable ADC interrupt
    NVIC_EnableIRQ(TA0_0_IRQn);  // Enable timer A0 interrupt
    NVIC_EnableIRQ(TA0_N_IRQn);  // Enable timer A0 overflow interrupt
    NVIC_EnableIRQ(TA1_0_IRQn);  // Enable timer A1 interrupt
    NVIC_EnableIRQ(PORT4_IRQn);  // Enable Port 4 interrupt (blinkers)
    NVIC_EnableIRQ(T32_INT1_IRQn);  // Enable Timer32 interrupt
    NVIC_EnableIRQ(PORT3_IRQn);  // Enable Port 3 interrupt (rotary encoder)
}

// Initialize all peripherals (called from main)
void initialize_peripherals(void) {
    initialize_ADC();  // Initialize ADC
    initialize_I2C();  // Initialize I2C
    initialize_SPI();  // Initialize SPI
    initialize_TIMERS();  // Initialize Timers
    rotaryInit();  // Initialize rotary encoder
    init_display();  // Initialize display
    enable_interrupts();  // Enable all peripheral interrupts
}

// Handle the ADC readings and update the sensor values
void handle_adc_readings(DashboardData *data) {
    data->adcValue = clamp(data->adcValue, 0, 16384);  // Adjust ADC value between 0 and 16384
    data->waterLevel = clamp(data->waterLevel, 2000, 8000);  // Clamp water level range

    TIMER_A2->CCR[3] = (4.87 * data->waterLevel) + 1067;  // Adjust PWM for water level gauge
    data->temperature = clamp(data->temperature, 2000, 6000);  // Clamp temperature range
    TIMER_A2->CCR[4] = (7.3 * data->temperature) - 3800;  // Adjust PWM for temperature gauge

    // Low fuel warning logic
    if ((data->waterLevel / 16384) <= 0.15 && !data->lowFuelFlag) {
        sendCharSPI(LOWFUEL);  // Send warning to slave
        data->lowFuelFlag = 1;  // Set flag to avoid sending repeatedly
    } else if ((data->waterLevel / 16384) > 0.15 && data->lowFuelFlag) {
        sendCharSPI(LOWFUEL);  // Reset warning on slave
        data->lowFuelFlag = 0;
    }

    // High temperature warning logic
    if ((data->temperature / 16384) >= 0.75 && !data->highTempFlag) {
        sendCharSPI(HIGHTEMP);  // Send warning to slave
        data->highTempFlag = 1;  // Set flag to avoid sending repeatedly
    } else if ((data->temperature / 16384) < 0.75 && data->highTempFlag) {
        sendCharSPI(HIGHTEMP);  // Reset warning on slave
        data->highTempFlag = 0;
    }
}

// Update speed and tachometer readings based on the captured period
void update_speed_and_tachometer(DashboardData *data) {
    // Compute actual speed based on the period between hall sensor pulses
    if (data->period <= 4211) data->actualSpeed = 100;
    else if (data->period >= 65535) data->actualSpeed = 0;
    else data->actualSpeed = ((3600 / (((float) data->period / 32788) * 2)) * 37.6991) / 5280;

    // Convert speed to gauge values
    data->currentSpeed = (data->actualSpeed / 100) * GAUGEMAX;
    data->tachValue = data->currentSpeed / 2 + IDLERPM;

    // Move the gauge needles to match the calculated values
    if (data->currentSpeed < data->tachValue) speedForward();
    else if (data->currentSpeed > data->tachValue) speedReverse();

    if (data->currentRPM < data->tachValue) tachForward();
    else if (data->currentRPM > data->tachValue) tachReverse();
}

// Update the odometer based on the calculated speed
void update_odometer(DashboardData *data) {
    data->odometerFloat += data->actualSpeed / 7200;  // Increment odometer based on speed
    if ((data->odometerFloat - (float)data->odometer) > 1) {
        data->odometer++;  // Increment odometer once 1 mile is traveled
        SevSegPrint(data->odometer);  // Update 7-segment display
        I2C_burstWrite(EEPROM_ADDR, 0, 1, (unsigned char*)&data->odometer);  // Store in EEPROM
    }
}

// Update the time and date from the RTC via I2C
void update_time_and_date(DashboardData *data) {
    unsigned char timeDateReadback[7];
    I2C_burstRead(CLOCK_ADDR, 0, 7, timeDateReadback);  // Read RTC values

    // Update date array (MM/DD/YY)
    data->dateArray[0] = (timeDateReadback[5] & 0x10) >> 4;  // Month tens
    data->dateArray[1] = timeDateReadback[5] & 0x0F;  // Month ones
    data->dateArray[2] = (timeDateReadback[4] & 0x30) >> 4;  // Day tens
    data->dateArray[3] = timeDateReadback[4] & 0x0F;  // Day ones
    data->dateArray[4] = (timeDateReadback[6] & 0xF0) >> 4;  // Year tens
    data->dateArray[5] = timeDateReadback[6] & 0x0F;  // Year ones

    // Update time array (HH:MM:SS)
    data->timeArray[0] = (timeDateReadback[2] & 0x30) >> 4;  // Hour tens
    data->timeArray[1] = timeDateReadback[2] & 0x0F;  // Hour ones
    data->timeArray[2] = (timeDateReadback[1] & 0x70) >> 4;  // Minute tens
    data->timeArray[3] = timeDateReadback[1] & 0x0F;  // Minute ones
    data->timeArray[4] = (timeDateReadback[0] & 0x70) >> 4;  // Second tens
    data->timeArray[5] = timeDateReadback[0] & 0x0F;  // Second ones
}