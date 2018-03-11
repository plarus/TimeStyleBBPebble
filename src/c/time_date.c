#include <pebble.h>
#include "time.h"
#include "settings.h"
#include "time_date.h"

// the date and time strings
char time_date_currentDayNum[3];
char time_date_currentWeekNum[3];
char time_date_currentSecondsNum[4];
char time_date_altClock[4];
char time_date_hours[3];
char time_date_minutes[3];
uint8_t time_date_currentDayName;
uint8_t time_date_currentMonth;

// c can't do true modulus on negative numbers, apparently
// from http://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
static int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

void time_date_update(struct tm* time_info) {

  if (clock_is_24h_style()) {
    strftime(time_date_hours, sizeof(time_date_hours), (globalSettings.showLeadingZero) ? "%H" : "%k", time_info);
  } else {
    strftime(time_date_hours, sizeof(time_date_hours), (globalSettings.showLeadingZero) ? "%I" : "%l", time_info);
  }

  if(time_date_hours[0] == ' ' && globalSettings.centerTime) {
    time_date_hours[0] = time_date_hours[1];
    time_date_hours[1] = '\0';
  }

  // minutes
  strftime(time_date_minutes, sizeof(time_date_minutes), "%M", time_info);

  // set all the date strings
  strftime(time_date_currentDayNum,  3, "%e", time_info);
  strftime(time_date_currentWeekNum, 3, "%V", time_info);

  // remove padding on date num, if needed
  if(time_date_currentDayNum[0] == ' ') {
    time_date_currentDayNum[0] = time_date_currentDayNum[1];
    time_date_currentDayNum[1] = '\0';
  }

  // set the seconds string
  strftime(time_date_currentSecondsNum, 4, ":%S", time_info);

  time_date_currentDayName = time_info->tm_wday;
  time_date_currentMonth = time_info->tm_mon;

  if(globalSettings.enableAltTimeZone) {
    // set the alternate time zone string
    int hour = time_info->tm_hour;

    // apply the configured offset value
    hour += globalSettings.altclockOffset;

    char am_pm;

    // format it
    if(clock_is_24h_style()) {
      hour = mod(hour, 24);
      am_pm = (char) 0;
    } else {
      hour = mod(hour, 12);
      if(hour == 0) {
        hour = 12;
      }
      am_pm = (mod(hour, 24) < 12) ? 'a' : 'p';
    }

    if(globalSettings.showLeadingZero && hour < 10) {
      snprintf(time_date_altClock, sizeof(time_date_altClock), "0%i%c", hour, am_pm);
    } else {
      snprintf(time_date_altClock, sizeof(time_date_altClock), "%i%c", hour, am_pm);
    }
  }
}
