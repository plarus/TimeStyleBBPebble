#pragma once
#include <pebble.h>
#include "sidebar_widgets.h"

#define SETTINGS_VERSION_KEY 4

// settings "version" for app version 4.0
#define CURRENT_SETTINGS_VERSION 8

#define FIXED_WIDGET_HEIGHT 51

#define LANGUAGE_EN 0
#define LANGUAGE_FR 1
#define LANGUAGE_DE 2
#define LANGUAGE_ES 3
#define LANGUAGE_IT 4
#define LANGUAGE_NL 5
#define LANGUAGE_TR 6
#define LANGUAGE_CZ 7
#define LANGUAGE_PT 8
#define LANGUAGE_GK 9
#define LANGUAGE_SE 10
#define LANGUAGE_PL 11
#define LANGUAGE_SK 12
#define LANGUAGE_VN 13
#define LANGUAGE_RO 14
#define LANGUAGE_CA 15
#define LANGUAGE_NO 16
#define LANGUAGE_RU 17
#define LANGUAGE_EE 18
#define LANGUAGE_EU 19
#define LANGUAGE_FI 20
#define LANGUAGE_DA 21
#define LANGUAGE_LT 22
#define LANGUAGE_SL 23
#define LANGUAGE_HU 24
#define LANGUAGE_HR 25
#define LANGUAGE_GA 26
#define LANGUAGE_LV 27
#define LANGUAGE_SR 28
#define LANGUAGE_CN 29
#define LANGUAGE_ID 30
#define LANGUAGE_UK 31
#define LANGUAGE_CY 32 // welsh
#define LANGUAGE_GL 33 // gallacian
#define LANGUAGE_JP 34 // japanese
#define LANGUAGE_KR 35 // korean
#define LANGUAGE_IW 36 // hebrew
#define LANGUAGE_BG 37 // bulgarian

typedef enum {
  NONE     = 0,
  LEFT     = 1,
  RIGHT    = 2,
  BOTTOM   = 3,
  TOP      = 4
} BarLocationType;

typedef enum {
  NO_VIBE              = 0,
  VIBE_EVERY_HOUR      = 1,
  VIBE_EVERY_HALF_HOUR = 2
} VibeIntervalType;

typedef enum {
  STEPS     = 0,
  DISTANCE  = 1,
  DURATION  = 2,
  KCALORIES = 3
} ActivityDisplayType;

typedef struct {
  // color settings
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;

  // general settings
  uint8_t languageId;
  char languageDayNames[7][8];
  char languageMonthNames[12][8];
  char languageWordForWeek[12];

  bool showLeadingZero;
  bool centerTime;
  uint8_t clockFontId;

  // vibration settings
  bool btVibe;
  VibeIntervalType hourlyVibe;

  // sidebar settings
  SidebarWidgetType widgets[4];
  BarLocationType sidebarLocation;
  bool useLargeFonts;
  bool activateDisconnectIcon;

  // metric or imperial unit
  bool useMetric;

  // battery meter widget settings
  bool showBatteryPct;
  bool disableAutobattery;

  // alt tz widget settings
  char altclockName[8];
  int altclockOffset;

  // health widget Settings
  ActivityDisplayType healthActivityDisplay;
  bool healthUseRestfulSleep;
  char decimalSeparator;

  // dynamic settings (calculated based the currently-selected widgets)
  bool disableWeather;
  bool updateScreenEverySecond;
  bool enableAutoBatteryWidget;
  bool enableBeats;
  bool enableAltTimeZone;

  // TODO: these shouldn't be dynamic
  GColor iconFillColor;
  GColor iconStrokeColor;
} Settings;


// !! all future settings should be added to the bottom of this structure
// !! do not remove fields from this structure, it will lead to unexpected behaviour
typedef struct {
  GColor timeColor;
  GColor timeBgColor;
  GColor sidebarColor;
  GColor sidebarTextColor;

  // general settings
  uint8_t languageId;
  uint8_t showLeadingZero:1;
  uint8_t clockFontId:7;

  // vibration settings
  uint8_t btVibe:1;
  int8_t hourlyVibe:7;

  // sidebar settings
  uint8_t widgets[4];
  uint8_t useLargeFonts:1;

  // weather widget settings
  uint8_t useMetric:1;

  // battery meter widget settings
  uint8_t showBatteryPct:1;
  uint8_t disableAutobattery:1;

  // health widget Settings
  ActivityDisplayType healthActivityDisplay:2;
  uint8_t healthUseRestfulSleep:1;
  char decimalSeparator;

  // alt tz widget settings
  char altclockName[8];
  int8_t altclockOffset;

  // sidebar location settings
  BarLocationType sidebarLocation:3;

  // bluetooth disconnection icon
  int8_t activateDisconnectIcon:1;

  int8_t centerTime:1;

  char languageDayNames[7][8];
  char languageMonthNames[12][8];
  char languageWordForWeek[12];
} StoredSettings;

extern Settings globalSettings;

// key for all the settings for versions 6 and higher
#define SETTING_VERSION6_AND_HIGHER       100

void Settings_init(void);
void Settings_deinit(void);
void Settings_saveToStorage(void);
void Settings_updateDynamicSettings(void);
