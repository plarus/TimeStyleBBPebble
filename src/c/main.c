#include <pebble.h>
#include "clock_area.h"
#include "messaging.h"
#include "settings.h"
#include "weather.h"
#include "sidebar.h"
#include "util.h"
#ifdef PBL_HEALTH
#include "health.h"
#endif
#include "time_date.h"
#include "debug.h"

// windows and layers
static Window* mainWindow;
static Layer* windowLayer;

// current bluetooth state
static bool isPhoneConnected;

// current time service subscription
static bool updatingEverySecond;

// try to randomize when watches call the weather API
static uint8_t weatherRefreshMinute;

static void update_clock(void) {
  time_t rawTime;
  struct tm* timeInfo;

  time(&rawTime);
  timeInfo = localtime(&rawTime);

  time_date_update(timeInfo);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // every 30 minutes, request new weather data
  if(!globalSettings.disableWeather) {
    if(tick_time->tm_min == weatherRefreshMinute && tick_time->tm_sec == 0) {
      messaging_requestNewWeatherData();
    }
  }

  // every hour, if requested, vibrate
  if(!quiet_time_is_active() && tick_time->tm_sec == 0) {
    if(globalSettings.hourlyVibe == VIBE_EVERY_HOUR) { // hourly vibes only
      if(tick_time->tm_min == 0) {
        vibes_double_pulse();
      }
    } else if(globalSettings.hourlyVibe == VIBE_EVERY_HALF_HOUR) {  // hourly and half-hourly
      if(tick_time->tm_min == 0) {
        vibes_double_pulse();
      } else if(tick_time->tm_min == 30) {
        vibes_short_pulse();
      }
    }
  }

  update_clock();

  // redraw all screen
  if(globalSettings.sidebarLocation != NONE) {
    Sidebar_redraw();
  }
  ClockArea_redraw();

  /* Debug */ Debug_display();
}

static void unobstructed_area_change_handler(AnimationProgress progress, void *context) {
  // update the sidebar
  Sidebar_redraw();
  /* Debug */ Debug_unobstructedAreaChange++;
}

static void unobstructed_area_did_change_handler(void *context) {
  // update the sidebar
  Sidebar_redraw();
  /* Debug */ Debug_unobstructedAreaChange++;
}

/* forces everything on screen to be redrawn -- perfect for keeping track of settings! */
static void redrawScreen() {

  // check if the tick handler frequency should be changed
  if(globalSettings.updateScreenEverySecond != updatingEverySecond) {
    tick_timer_service_unsubscribe();

    if(globalSettings.updateScreenEverySecond) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      updatingEverySecond = true;
    } else {
      tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
      updatingEverySecond = false;
    }

  }

  unobstructed_area_service_unsubscribe();

  if(globalSettings.sidebarLocation == TOP) {
    UnobstructedAreaHandlers unobstructed_area_handlers = {
      .change = unobstructed_area_change_handler,
      .did_change = unobstructed_area_did_change_handler
    };

    unobstructed_area_service_subscribe(unobstructed_area_handlers, NULL);
  }

  window_set_background_color(mainWindow, globalSettings.timeBgColor);

  // maybe the language changed!
  update_clock();

  // maybe sidebar changed!
  Sidebar_set_layer();

  // update the sidebar
  Sidebar_redraw();

  // check if the fonts need to be switched
  ClockArea_update_fonts();

  ClockArea_redraw();

  /* Debug */ Debug_RedrawFunction++;
}

static void main_window_load(Window *window) {
  window_set_background_color(window, globalSettings.timeBgColor);

  // create the sidebar
  Sidebar_init(window);

  ClockArea_init(window);

  // Make sure the time is displayed from the start
  redrawScreen();
}

static void main_window_unload(Window *window) {
  ClockArea_deinit();
  Sidebar_deinit();
}

static void bluetoothStateChanged(bool newConnectionState) {
  // if the phone was connected but isn't anymore and the user has opted in,
  // trigger a vibration
  if(!quiet_time_is_active() && isPhoneConnected && !newConnectionState && globalSettings.btVibe) {
    static uint32_t const segments[] = { 200, 100, 100, 100, 500 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
      };
    vibes_enqueue_custom_pattern(pat);
  }

  // if the phone was disconnected and isn't anymore, update the data
  if(!isPhoneConnected && newConnectionState) {
    messaging_requestNewWeatherData();
  }

  isPhoneConnected = newConnectionState;

  if(globalSettings.sidebarLocation != NONE) {
    Sidebar_redraw();
  }
  /* Debug */ Debug_bluetoothStateChange++;
}

// force the sidebar to redraw any time the battery state changes
static void batteryStateChanged(BatteryChargeState charge_state) {
  if(globalSettings.sidebarLocation != NONE) {
    Sidebar_redraw();
  }
  /* Debug */ Debug_batteryHandler++;
  /* Debug */ Debug_chargePercent = charge_state.charge_percent;
}

// fixes for disappearing elements after notifications
// (from http://codecorner.galanter.net/2016/01/08/solved-issue-with-pebble-framebuffer-after-notification-is-dismissed/)
static void app_focus_changing(bool focusing) {
  if (focusing) {
     layer_set_hidden(windowLayer, true);
     /* Debug */ Debug_focusChange++;
  }
}

static void app_focus_changed(bool focused) {
  if (focused) {
     layer_set_hidden(windowLayer, false);
     layer_mark_dirty(windowLayer);
     /* Debug */ Debug_focusChange++;
  }
}

static void init(void) {
  setlocale(LC_ALL, "");

  srand(time(NULL));

  weatherRefreshMinute = rand() % 60;

  // init settings
  Settings_init();

  // init weather system
  Weather_init();

#ifdef PBL_HEALTH
  // init health service
  if (!Health_init(Sidebar_redraw)) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Could not init health service");
  }
#endif

  // init the messaging thing
  messaging_init(redrawScreen);

  // Create main Window element and assign to pointer
  mainWindow = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(mainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(mainWindow, true);

  windowLayer = window_get_root_layer(mainWindow);

  // Register with TickTimerService
  if(globalSettings.updateScreenEverySecond) {
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    updatingEverySecond = true;
  } else {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    updatingEverySecond = false;
  }

  bool connected = bluetooth_connection_service_peek();
  bluetoothStateChanged(connected);
  bluetooth_connection_service_subscribe(bluetoothStateChanged);

  // register with battery service
  battery_state_service_subscribe(batteryStateChanged);

  // set up focus change handlers
  app_focus_service_subscribe_handlers((AppFocusHandlers){
    .did_focus = app_focus_changed,
    .will_focus = app_focus_changing
  });
}

static void deinit(void) {
  // Destroy Window
  window_destroy(mainWindow);

#ifdef PBL_HEALTH
  Health_deinit();
#endif
  // unload weather stuff
  Weather_deinit();
  Settings_deinit();

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  unobstructed_area_service_unsubscribe();

  /* Debug */ Debug_display();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
