#ifndef SENSORS_H
#define SENSORS_H

// Define pins and ports for readability and maintainability
#define HALLPORT P2  // Hall sensor port
#define HALLPIN  BIT4  // Hall sensor pin

#define ADCPORT P4  // ADC port for sensors
#define ADCPIN  BIT5  // ADC pin
#define WATERPIN BIT6  // Water sensor pin
#define TEMPPIN BIT7  // Temperature sensor pin

#define SERVOPORT P6  // Servo motors port
#define WATERGAUGE BIT6  // Water gauge servo pin
#define TEMPGAUGE BIT7  // Temp gauge servo pin

// Structure to hold all sensor and dashboard-related data
typedef struct {
    float actualSpeed;
    int currentSpeed;
    int tachValue;
    int currentRPM;
    float adcValue;
    float waterLevel;
    float temperature;
    int ADCFlag;
    int masterTimer;
    int lowFuelFlag;
    int highTempFlag;
    float chipTemp;
    char timeArray[6];
    char dateArray[6];
    int period;
    int odometer;
    float odometerFloat;
} DashboardData;

// Function declarations for sensors and peripheral management
void handle_adc_readings(DashboardData *data);  // Process ADC readings
void update_speed_and_tachometer(DashboardData *data);  // Update speed and RPM values
void update_odometer(DashboardData *data);  // Update the odometer
void initialize_peripherals(void);  // Initialize all peripherals
void update_time_and_date(DashboardData *data);  // Update the time and date from RTC
void enable_interrupts(void);  // Enable interrupts for all peripherals

#endif