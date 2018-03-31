#pragma once
#include <pebble.h>

// the date and time strings
extern char time_date_currentDayNum[3];
extern char time_date_currentWeekNum[3];
extern char time_date_currentSecondsNum[4];
extern char time_date_altClock[4];
extern char time_date_hours[3];
extern char time_date_minutes[3];
extern uint8_t time_date_currentDayName;
extern uint8_t time_date_currentMonth;

void time_date_update(void);
