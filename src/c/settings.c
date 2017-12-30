#include <pebble.h>
#include "languages.h"
#include "settings.h"
#include "clock_area.h"

Settings globalSettings;

void Settings_init(void) {
  // first, check if we have any saved settings
  int current_settings_version = persist_exists(SETTINGS_VERSION_KEY) ? persist_read_int(SETTINGS_VERSION_KEY) : -1;
  APP_LOG(APP_LOG_LEVEL_DEBUG,"current_settings_version: %d", current_settings_version);
  if( current_settings_version <= 6 ) {
  //int settingsVersion = persist_read_int(SETTINGS_VERSION_KEY);
    Settings_loadDefaultsSettings();
  } else {
    // load all settings
    Settings_loadFromStorage();
  }
  Settings_updateDynamicSettings();
}

void Settings_deinit(void) {
  // ensure that the weather disabled setting is accurate before saving it
  Settings_updateDynamicSettings();

  // write all settings to storage
  Settings_saveToStorage();
}

/*
 * Load defaults settings
 */
void Settings_loadDefaultsSettings(void) {

  globalSettings.timeColor        = GColorWhite;
  globalSettings.sidebarColor     = GColorLightGray;
  globalSettings.timeBgColor      = GColorBlack;
  globalSettings.sidebarTextColor = GColorBlack;

  globalSettings.languageId       = LANGUAGE_EN; // English
  globalSettings.showLeadingZero  = false;
  globalSettings.clockFontId      = FONT_SETTING_DEFAULT;
  globalSettings.btVibe           = false;
  globalSettings.hourlyVibe       = NO_VIBE;
  globalSettings.sidebarLocation  = RIGHT;

  // set the default widgets
  globalSettings.widgets[0] = BATTERY_METER;
  globalSettings.widgets[1] = EMPTY;
  globalSettings.widgets[2] = DATE;
  globalSettings.widgets[3] = EMPTY;

  globalSettings.useLargeFonts          = false;
  globalSettings.useMetric              = true;
  globalSettings.showBatteryPct         = true;
  globalSettings.disableAutobattery     = false;
  strncpy(globalSettings.altclockName, "ALT", sizeof(globalSettings.altclockName));
  globalSettings.altclockOffset         = 0;
  globalSettings.activateDisconnectIcon = false; //TODO: Set to true;
}

/*
 * Load the saved color settings
 */
void Settings_loadFromStorage(void) {
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
    globalSettings.widgets[3] = storedSettings.widgets[3];
    globalSettings.useLargeFonts = storedSettings.useLargeFonts;
    globalSettings.useMetric = storedSettings.useMetric;
    globalSettings.showBatteryPct = storedSettings.showBatteryPct;
    globalSettings.disableAutobattery = storedSettings.disableAutobattery;
    memcpy(globalSettings.altclockName, storedSettings.altclockName, 8);
    globalSettings.altclockOffset = storedSettings.altclockOffset;
    globalSettings.activateDisconnectIcon = storedSettings.activateDisconnectIcon;
}

void Settings_saveToStorage(void) {
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
  storedSettings.widgets[3] = globalSettings.widgets[3];
  storedSettings.useLargeFonts = globalSettings.useLargeFonts;
  storedSettings.useMetric = globalSettings.useMetric;
  storedSettings.showBatteryPct = globalSettings.showBatteryPct;
  storedSettings.disableAutobattery = globalSettings.disableAutobattery;
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
  globalSettings.enableAltTimeZone = false;
  globalSettings.prev_clockFontId = FONT_SETTING_UNSET;

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
