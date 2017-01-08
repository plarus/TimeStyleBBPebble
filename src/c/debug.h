#pragma once
#include <pebble.h>

extern int Debug_sidebarUpdate;
extern int Debug_clockAreaUpdate;

extern int16_t Debug_RedrawFunction;
extern int Debug_healthEventHandlerCall;
extern int16_t Debug_healthSignificantUpdate;
extern int Debug_healthSleepCall;
extern int16_t Debug_movSleepUpdate;
extern int Debug_restfulSleeping;
extern int Debug_walking;
extern int Debug_running;
extern int16_t Debug_focusChange;
extern int16_t Debug_bluetoothStateChange;
extern int16_t Debug_unobstructedAreaChange;

/*
 * Display all debug values
 */
void Debug_display(void);
