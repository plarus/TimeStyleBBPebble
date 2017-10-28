#ifdef PBL_HEALTH
#include <pebble.h>
#include "health.h"

#define SECONDS_AFTER_WAKE_UP 1800 // Half hour

static bool s_sleeping;
static bool s_restfulSleeping;
static time_t s_endSleepTime;
static HealthValue s_sleep_seconds;
static HealthValue s_restful_sleep_seconds;
static HealthValue s_distance_walked;
static HealthValue s_steps;
static HealthValue s_active_seconds;
static HealthValue s_active_kCalories;

static inline bool is_health_metric_accessible(HealthMetric metric, time_t time_start, time_t time_end) {
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, time_start, time_end);
    return mask & HealthServiceAccessibilityMaskAvailable;
}

static inline HealthValue get_health_value_sum_today(HealthMetric metric) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);

    return is_health_metric_accessible(metric, start, end) ? health_service_sum_today(metric) : 0;
}

void Health_update(void) {
    HealthActivityMask mask = health_service_peek_current_activities();

    // Sleep
    s_sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
    s_restfulSleeping = (mask & HealthActivityRestfulSleep);
    s_sleep_seconds = get_health_value_sum_today(HealthMetricSleepSeconds);
    s_restful_sleep_seconds = get_health_value_sum_today(HealthMetricSleepRestfulSeconds);

    if(s_sleeping) {
        s_endSleepTime = time(NULL);
    }

    // Steps
    s_distance_walked = get_health_value_sum_today(HealthMetricWalkedDistanceMeters);
    s_steps = get_health_value_sum_today(HealthMetricStepCount);
    s_active_seconds = get_health_value_sum_today(HealthMetricActiveSeconds);
    s_active_kCalories = get_health_value_sum_today(HealthMetricActiveKCalories);
}

bool Health_isUserSleeping(void) {
    return s_sleeping;
}

bool Health_isUserRestfulSleeping(void) {
    return s_restfulSleeping;
}

bool Health_sleepingToBeDisplayed(void) {
    // Sleep should be display during an half hour after wake up
    return s_sleeping || (s_endSleepTime + SECONDS_AFTER_WAKE_UP > time(NULL));
}

HealthValue Health_getSleepSeconds(void) {
    return s_sleep_seconds;
}

HealthValue Health_getRestfulSleepSeconds(void) {
    return s_restful_sleep_seconds;
}

HealthValue Health_getDistanceWalked(void) {
    return s_distance_walked;
}

HealthValue Health_getSteps(void) {
    return s_steps;
}

HealthValue Health_getActiveSeconds(void) {
    return s_active_seconds;
}

HealthValue Health_getActiveKCalories(void) {
    return s_active_kCalories;
}

#endif // PBL_HEALTH
