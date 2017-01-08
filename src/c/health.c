#ifdef PBL_HEALTH
#include <pebble.h>
#include "health.h"
#include "debug.h"

static HealthEventCallback s_health_event_callback;


#define SECONDS_AFTER_WAKE_UP 1800 // Half hour

static bool s_sleeping;
static bool s_restfulSleeping;
static bool s_walking;
static bool s_running;
static time_t s_endSleepTime;
static HealthValue s_sleep_seconds;
static HealthValue s_restful_sleep_seconds;
static HealthValue s_distance_walked;
static HealthValue s_steps;
static HealthValue s_heart_rate;

static inline bool is_health_metric_accessible(HealthMetric metric, time_t time_start, time_t time_end) {
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, time_start, time_end);
    return mask & HealthServiceAccessibilityMaskAvailable;
}

static inline HealthValue get_health_value_sum_today(HealthMetric metric) {
    time_t start = time_start_of_today();
    time_t end = time(NULL);

    return is_health_metric_accessible(metric, start, end) ? health_service_sum_today(metric) : 0;
}

static void health_event_handler(HealthEventType event, void *context) {
    if (event == HealthEventSignificantUpdate) {
        health_event_handler(HealthEventSleepUpdate, &event);
        health_event_handler(HealthEventMovementUpdate, &event);
        health_event_handler(HealthEventHeartRateUpdate, &event);
        /* Debug */ Debug_healthSignificantUpdate++;
    } else if (event == HealthEventSleepUpdate) {
        HealthActivityMask mask = health_service_peek_current_activities();
        s_sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
        if (s_sleeping) {
            s_endSleepTime = time(NULL);
        }
        s_restfulSleeping = (mask & HealthActivityRestfulSleep);
        if (s_restfulSleeping) {
            /* Debug */ Debug_restfulSleeping++;
        }
        s_sleep_seconds = get_health_value_sum_today(HealthMetricSleepSeconds);
        s_restful_sleep_seconds = get_health_value_sum_today(HealthMetricSleepRestfulSeconds);
        /* Debug */ Debug_healthSleepCall++;
    } else if (event == HealthEventMovementUpdate) {
        HealthActivityMask mask = health_service_peek_current_activities();
        s_sleeping = (mask & HealthActivitySleep) || (mask & HealthActivityRestfulSleep);
        if (s_sleeping) {
            s_endSleepTime = time(NULL);
            /* Debug */ Debug_movSleepUpdate++;
        }
        s_walking = mask & HealthActivityWalk;
        if (s_walking) {
            /* Debug */ Debug_walking++;
        }
        s_running = mask & HealthActivityRun;
        if (s_running) {
            /* Debug */ Debug_running++;
        }
        s_distance_walked = get_health_value_sum_today(HealthMetricWalkedDistanceMeters);
        s_steps = get_health_value_sum_today(HealthMetricStepCount);
    } else if (event == HealthEventHeartRateUpdate) {
        time_t now = time(NULL);

        if (is_health_metric_accessible(HealthMetricHeartRateBPM, now, now)) {
            s_heart_rate = health_service_peek_current_value(HealthMetricHeartRateBPM);
        }
    }

    // Execute callback only one time if significant update
    if ((event != HealthEventSignificantUpdate && !context) ||
        (event == HealthEventHeartRateUpdate && context)) {
      if (s_health_event_callback) s_health_event_callback();
      /* Debug */ Debug_healthEventHandlerCall++;
    }
}

bool Health_init(HealthEventCallback callback) {
    bool b = health_service_events_subscribe(health_event_handler, NULL);
    if (b) {
        health_event_handler(HealthEventSignificantUpdate, NULL);
        s_health_event_callback = callback;
    }
    return b;
}

void Health_deinit(void) {
    health_service_events_unsubscribe();
}

bool Health_isUserSleeping(void) {
    return s_sleeping;
}

bool Health_isUserRestfulSleeping(void) {
    return s_restfulSleeping;
}

bool Health_sleepingToBeDisplayed(void) {
    return s_sleeping || (s_endSleepTime + SECONDS_AFTER_WAKE_UP > time(NULL));
}

bool Health_isUserWalking(void) {
    return s_walking;
}

bool Health_isUserRunning(void) {
    return s_running;
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

HealthValue Health_getHeartRate(void) {
    return s_heart_rate;
}

#endif // PBL_HEALTH
