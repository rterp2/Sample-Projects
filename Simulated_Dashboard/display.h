#ifndef DISPLAY_H
#define DISPLAY_H

#include "sensors.h"

// Function declarations for display management
void update_display(DashboardData *data);  // Update all displayed information
void update_speed_on_display(int speed);   // Update speed display
void update_time_on_display(char* time);   // Update time display
void update_date_on_display(char* date);   // Update date display
void init_display(void);  // Initialize the display

#endif