#pragma once
#include <pebble.h>

typedef void (*HealthEventCallback)(void);

bool Health_init(HealthEventCallback callback);
void Health_deinit(void);
bool Health_isUserSleeping(void);
bool Health_isUserRestfulSleeping(void);
bool Health_sleepingToBeDisplayed(void);
bool Health_isUserWalking(void);
bool Health_isUserRunning(void);
HealthValue Health_getSleepSeconds(void);
HealthValue Health_getRestfulSleepSeconds(void);
HealthValue Health_getDistanceWalked(void);
HealthValue Health_getSteps(void);
HealthValue Health_getHeartRate(void);
