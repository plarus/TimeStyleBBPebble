#include <pebble.h>
#include "settings.h"

Settings globalSettings;

void Settings_init(void) {
  // first, check if we have any saved settings
  // int settingsVersion = persist_read_int(SETTINGS_VERSION_KEY);

  // load all settings
  Settings_loadFromStorage();
}

void Settings_deinit(void) {
  // write all settings to storage
  Settings_saveToStorage();
}

/*
 * Load the saved color settings, or if they don't exist load defaults
 */
void Settings_loadFromStorage(void) {
  // load the default colors
  globalSettings.timeBgColor      = GColorBlack;
  globalSettings.sidebarTextColor = GColorBlack;

  #ifdef PBL_COLOR
    globalSettings.timeColor      = GColorOrange;
    globalSettings.sidebarColor   = GColorOrange;
  #else
    globalSettings.timeColor      = GColorWhite;
    globalSettings.sidebarColor   = GColorLightGray;
  #endif

  // set the default widgets
  globalSettings.widgets[0] = PBL_IF_HEALTH_ELSE(HEALTH, BATTERY_METER);
  globalSettings.widgets[1] = EMPTY;
  globalSettings.widgets[2] = DATE;
  globalSettings.widgets[3] = EMPTY;

  // set default bar display
  globalSettings.sidebarLocation = RIGHT;

  // set default disconnect icon activation
  globalSettings.activateDisconnectIcon = true;

  strncpy(globalSettings.altclockName, "ALT", sizeof(globalSettings.altclockName));
  globalSettings.decimalSeparator = '.';

  globalSettings.showBatteryPct = true;

  int current_settings_version = persist_exists(SETTINGS_VERSION_KEY) ? persist_read_int(SETTINGS_VERSION_KEY) : -1;
  APP_LOG(APP_LOG_LEVEL_DEBUG,"current_settings_version: %d", current_settings_version);
  if( current_settings_version > 5 ) {
    StoredSettings storedSettings;
    memset(&storedSettings,0,sizeof(StoredSettings));
    // if previous version settings are used than only first part of settings would be overwritten,
    // all the other fields will left filled with zeroes
    persist_read_data(SETTING_VERSION6_AND_HIGHER, &storedSettings, sizeof(StoredSettings));
    globalSettings.timeColor = storedSettings.timeColor;
    globalSettings.timeBgColor = storedSettings.timeBgColor;
    globalSettings.sidebarColor = storedSettings.sidebarColor;
    globalSettings.sidebarTextColor = storedSettings.sidebarTextColor;
    globalSettings.languageId = storedSettings.languageId;
    globalSettings.showLeadingZero = storedSettings.showLeadingZero;
    globalSettings.clockFontId = storedSettings.clockFontId;
    globalSettings.btVibe = storedSettings.btVibe;
    globalSettings.hourlyVibe = storedSettings.hourlyVibe;
    globalSettings.sidebarLocation = storedSettings.sidebarLocation;
    globalSettings.widgets[0] = storedSettings.widgets[0];
    globalSettings.widgets[1] = storedSettings.widgets[1];
    globalSettings.widgets[2] = storedSettings.widgets[2];
    if (globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
      globalSettings.widgets[3] = storedSettings.widget4;
    }
    globalSettings.useLargeFonts = storedSettings.useLargeFonts;
    globalSettings.useMetric = storedSettings.useMetric;
    globalSettings.showBatteryPct = storedSettings.showBatteryPct;
    globalSettings.disableAutobattery = storedSettings.disableAutobattery;

    if (storedSettings.healthUseDistance) {
      globalSettings.healthActivityDisplay = A_DISTANCE;
    } else {
      globalSettings.healthActivityDisplay = A_STEPS;
    }
    globalSettings.healthUseRestfulSleep = storedSettings.healthUseRestfulSleep;
    globalSettings.decimalSeparator = storedSettings.decimalSeparator;
    memcpy(globalSettings.altclockName, storedSettings.altclockName, 8);
    globalSettings.altclockOffset = storedSettings.altclockOffset;
    globalSettings.activateDisconnectIcon = storedSettings.activateDisconnectIcon;
  }

  Settings_updateDynamicSettings();
}

void Settings_saveToStorage(void) {
  // ensure that the weather disabled setting is accurate before saving it
  Settings_updateDynamicSettings();

  // save settings to compressed structure and to persistent storage
  StoredSettings storedSettings;
  // if previous version settings are used than only first part of settings would be overwrited
  // all the other fields will left filled with zeroes
  storedSettings.timeColor = globalSettings.timeColor;
  storedSettings.timeBgColor = globalSettings.timeBgColor;
  storedSettings.sidebarColor = globalSettings.sidebarColor;
  storedSettings.sidebarTextColor = globalSettings.sidebarTextColor;
  storedSettings.languageId = globalSettings.languageId;
  storedSettings.showLeadingZero = globalSettings.showLeadingZero;
  storedSettings.clockFontId = globalSettings.clockFontId;
  storedSettings.btVibe = globalSettings.btVibe;
  storedSettings.hourlyVibe = globalSettings.hourlyVibe;
  storedSettings.widgets[0] = globalSettings.widgets[0];
  storedSettings.widgets[1] = globalSettings.widgets[1];
  storedSettings.widgets[2] = globalSettings.widgets[2];
  storedSettings.widget4    = globalSettings.widgets[3];
  storedSettings.useLargeFonts = globalSettings.useLargeFonts;
  storedSettings.useMetric = globalSettings.useMetric;
  storedSettings.showBatteryPct = globalSettings.showBatteryPct;
  storedSettings.disableAutobattery = globalSettings.disableAutobattery;
  if (globalSettings.healthActivityDisplay == A_DISTANCE) {
    storedSettings.healthUseDistance = true;
  } else {
    storedSettings.healthUseDistance = false;
  }
  storedSettings.healthUseRestfulSleep = globalSettings.healthUseRestfulSleep;
  storedSettings.decimalSeparator = globalSettings.decimalSeparator;
  memcpy(storedSettings.altclockName, globalSettings.altclockName, 8);
  storedSettings.altclockOffset = globalSettings.altclockOffset;
  storedSettings.sidebarLocation = globalSettings.sidebarLocation;
  storedSettings.activateDisconnectIcon = globalSettings.activateDisconnectIcon;

  persist_write_data(SETTING_VERSION6_AND_HIGHER, &storedSettings, sizeof(StoredSettings));
  persist_write_int(SETTINGS_VERSION_KEY, CURRENT_SETTINGS_VERSION);
}

void Settings_updateDynamicSettings(void) {
  globalSettings.disableWeather = true;
  globalSettings.updateScreenEverySecond = false;
  globalSettings.enableAutoBatteryWidget = true;
  globalSettings.enableBeats = false;
  globalSettings.enableAltTimeZone = false;

  for(int i = 0; i < 4; i++) {
    // if there are any weather widgets, enable weather checking
    // if(globalSettings.widgets[i] == WEATHER_CURRENT ||
    //    globalSettings.widgets[i] == WEATHER_FORECAST_TODAY) {
    if(globalSettings.widgets[i] == WEATHER_CURRENT) {
      globalSettings.disableWeather = false;
    }

    // if any widget is "seconds", we'll need to update the sidebar every second
    if(globalSettings.widgets[i] == SECONDS) {
      globalSettings.updateScreenEverySecond = true;
    }

    // if any widget is "battery", disable the automatic battery indication
    if(globalSettings.widgets[i] == BATTERY_METER) {
      globalSettings.enableAutoBatteryWidget = false;
    }

    // if any widget is "beats", enable the beats calculation
    if(globalSettings.widgets[i] == BEATS) {
      globalSettings.enableBeats = true;
    }

    // if any widget is "alt_time_zone", enable the alternative time calculation
    if(globalSettings.widgets[i] == ALT_TIME_ZONE) {
      globalSettings.enableAltTimeZone = true;
    }
  }

  // temp: if the sidebar is black, use inverted colors for icons
  if(gcolor_equal(globalSettings.sidebarColor, GColorBlack)) {
    globalSettings.iconFillColor = GColorBlack;
    globalSettings.iconStrokeColor = globalSettings.sidebarTextColor; // exciting
  } else {
    globalSettings.iconFillColor = GColorWhite;
    globalSettings.iconStrokeColor = GColorBlack;
  }
}
