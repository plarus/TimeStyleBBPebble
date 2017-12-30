#pragma once
#include <pebble.h>

// the date and time strings
extern char time_date_currentDayName[8];
extern char time_date_currentDayNum[5];
extern char time_date_currentMonth[8];
extern char time_date_currentWeekNum[5];
extern char time_date_currentSecondsNum[5];
extern char time_date_altClock[8];
extern char time_date_currentBeats[5];
extern char time_date_hours[3];
extern char time_date_minutes[3];
#ifndef PBL_ROUND
extern char time_date_currentDate[21];
extern bool time_date_isAmHour;
#endif // PBL_ROUND

void time_date_update(struct tm* time_info);
