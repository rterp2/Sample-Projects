#include "display.h"
#include "ST7735.h"
#include <stdio.h>

// Update all display values: speed, date, time, and temperature
void update_display(DashboardData *data) {
    char buffer[10];

    // Display speed (e.g., "70 MPH")
    sprintf(buffer, "%d MPH", (int)data->actualSpeed);
    ST7735_DrawString(1, 2, buffer, ST7735_WHITE);

    // Display date (e.g., "12/24/23")
    char date[9];
    sprintf(date, "%02d/%02d/%02d", data->dateArray[0], data->dateArray[1], data->dateArray[2]);
    ST7735_DrawString(0, 10, date, ST7735_WHITE);

    // Display time (e.g., "12:00:30")
    char time[9];
    sprintf(time, "%02d:%02d:%02d", data->timeArray[0], data->timeArray[1], data->timeArray[2]);
    ST7735_DrawString(0, 12, time, ST7735_WHITE);

    // Display temperature (e.g., "22.5 C")
    char temp[6];
    sprintf(temp, "%.2f C", data->chipTemp);
    ST7735_DrawString(0, 14, temp, ST7735_WHITE);
}

// Initialize the display
void init_display(void) {
    ST7735_InitR(INITR_REDTAB);
}