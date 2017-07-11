#pragma once
#include <pebble.h>

void Health_update(void);
bool Health_isUserSleeping(void);
bool Health_isUserRestfulSleeping(void);
bool Health_sleepingToBeDisplayed(void);
HealthValue Health_getSleepSeconds(void);
HealthValue Health_getRestfulSleepSeconds(void);
HealthValue Health_getDistanceWalked(void);
HealthValue Health_getSteps(void);
HealthValue Health_getActiveSeconds(void);
HealthValue Health_getActiveKCalories(void);
HealthValue Health_getHeartRate(void);
