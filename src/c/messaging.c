#include <pebble.h>
#include "weather.h"
#include "settings.h"
#include "messaging.h"

static MessageProcessedCallback message_processed_callback;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // does this message contain current weather conditions?
  Tuple *weatherTemp_tuple = dict_find(iterator, MESSAGE_KEY_WeatherTemperature);
  Tuple *weatherConditions_tuple = dict_find(iterator, MESSAGE_KEY_WeatherCondition);

  if(weatherTemp_tuple != NULL && weatherConditions_tuple != NULL) {
    // now set the weather conditions properly
    Weather_weatherInfo.currentTemp = (int)weatherTemp_tuple->value->int32;

    Weather_setCurrentCondition(weatherConditions_tuple->value->int32);

    Weather_saveData();
  }

  // does this message contain weather forecast information?
  Tuple *weatherForecastCondition_tuple = dict_find(iterator, MESSAGE_KEY_WeatherForecastCondition);
  Tuple *weatherForecastHigh_tuple = dict_find(iterator, MESSAGE_KEY_WeatherForecastHighTemp);
  Tuple *weatherForecastLow_tuple = dict_find(iterator, MESSAGE_KEY_WeatherForecastLowTemp);

  if(weatherForecastCondition_tuple != NULL && weatherForecastHigh_tuple != NULL
     && weatherForecastLow_tuple != NULL) {

    Weather_weatherForecast.highTemp = (int)weatherForecastHigh_tuple->value->int32;
    Weather_weatherForecast.lowTemp = (int)weatherForecastLow_tuple->value->int32;
    Weather_setForecastCondition(weatherForecastCondition_tuple->value->int32);

    Weather_saveData();
  }

  // does this message contain new config information?
  Tuple *timeColor_tuple = dict_find(iterator, MESSAGE_KEY_SettingColorTime);
  Tuple *bgColor_tuple = dict_find(iterator, MESSAGE_KEY_SettingColorBG);
  Tuple *sidebarColor_tuple = dict_find(iterator, MESSAGE_KEY_SettingColorSidebar);
  Tuple *sidebarPos_tuple = dict_find(iterator, MESSAGE_KEY_SettingSidebarPosition);
  Tuple *sidebarTextColor_tuple = dict_find(iterator, MESSAGE_KEY_SettingSidebarTextColor);
  Tuple *useMetric_tuple = dict_find(iterator, MESSAGE_KEY_SettingUseMetric);
  Tuple *btVibe_tuple = dict_find(iterator, MESSAGE_KEY_SettingBluetoothVibe);
  Tuple *language_tuple = dict_find(iterator, MESSAGE_KEY_SettingLanguageID);
  Tuple *leadingZero_tuple = dict_find(iterator, MESSAGE_KEY_SettingShowLeadingZero);
  Tuple *batteryPct_tuple = dict_find(iterator, MESSAGE_KEY_SettingShowBatteryPct);
  Tuple *disableWeather_tuple = dict_find(iterator, MESSAGE_KEY_SettingDisableWeather);
  Tuple *clockFont_tuple = dict_find(iterator, MESSAGE_KEY_SettingClockFontId);
  Tuple *hourlyVibe_tuple = dict_find(iterator, MESSAGE_KEY_SettingHourlyVibe);
  Tuple *useLargeFonts_tuple = dict_find(iterator, MESSAGE_KEY_SettingUseLargeFonts);

  Tuple *widget0Id_tuple = dict_find(iterator, MESSAGE_KEY_SettingWidget0ID);
  Tuple *widget1Id_tuple = dict_find(iterator, MESSAGE_KEY_SettingWidget1ID);
  Tuple *widget2Id_tuple = dict_find(iterator, MESSAGE_KEY_SettingWidget2ID);
  Tuple *widget3Id_tuple = dict_find(iterator, MESSAGE_KEY_SettingWidget3ID);

  Tuple *altclockName_tuple = dict_find(iterator, MESSAGE_KEY_SettingAltClockName);
  Tuple *altclockOffset_tuple = dict_find(iterator, MESSAGE_KEY_SettingAltClockOffset);

  Tuple *autobattery_tuple = dict_find(iterator, MESSAGE_KEY_SettingDisableAutobattery);

  Tuple *activateDisconnectIcon_tuple = dict_find(iterator, MESSAGE_KEY_SettingDisconnectIcon);


  if(timeColor_tuple != NULL) {
    globalSettings.timeColor = GColorFromHEX(timeColor_tuple->value->int32);
  }

  if(bgColor_tuple != NULL) {
    globalSettings.timeBgColor = GColorFromHEX(bgColor_tuple->value->int32);
  }

  if(sidebarColor_tuple != NULL) {
    globalSettings.sidebarColor = GColorFromHEX(sidebarColor_tuple->value->int32);
  }

  if(sidebarTextColor_tuple != NULL) {
    // text can only be black or white, so we'll enforce that here
    globalSettings.sidebarTextColor = GColorFromHEX(sidebarTextColor_tuple->value->int32);
  }

  if(sidebarPos_tuple != NULL) {
    globalSettings.sidebarLocation = (BarLocationType)sidebarPos_tuple->value->int8;
  }

  if(useMetric_tuple != NULL) {
    globalSettings.useMetric = (bool)useMetric_tuple->value->int8;
  }

  if(btVibe_tuple != NULL) {
    globalSettings.btVibe = (bool)btVibe_tuple->value->int8;
  }

  if(leadingZero_tuple != NULL) {
    globalSettings.showLeadingZero = (bool)leadingZero_tuple->value->int8;
  }

  if(batteryPct_tuple != NULL) {
    globalSettings.showBatteryPct = (bool)batteryPct_tuple->value->int8;
  }

  if(autobattery_tuple != NULL) {
    globalSettings.disableAutobattery = (bool)autobattery_tuple->value->int8;
  }

  if(disableWeather_tuple != NULL) {
    globalSettings.disableWeather = (bool)disableWeather_tuple->value->int8;
  }

  if(clockFont_tuple != NULL) {
    globalSettings.clockFontId = clockFont_tuple->value->int8;
  }

  if(useLargeFonts_tuple != NULL) {
    globalSettings.useLargeFonts = (bool)useLargeFonts_tuple->value->int8;
  }

  if(hourlyVibe_tuple != NULL) {
    globalSettings.hourlyVibe = hourlyVibe_tuple->value->int8;
  }

  if(language_tuple != NULL) {
    globalSettings.languageId = language_tuple->value->int8;
  }

  if(widget0Id_tuple != NULL) {
    globalSettings.widgets[0] = widget0Id_tuple->value->int8;
  }

  if(widget1Id_tuple != NULL) {
    globalSettings.widgets[1] = widget1Id_tuple->value->int8;
  }

  if(widget2Id_tuple != NULL) {
    globalSettings.widgets[2] = widget2Id_tuple->value->int8;
  }

  if(widget3Id_tuple != NULL) {
    globalSettings.widgets[3] = widget3Id_tuple->value->int8;
  }

  if(altclockName_tuple != NULL) {
    strncpy(globalSettings.altclockName, altclockName_tuple->value->cstring, sizeof(globalSettings.altclockName));
  }

  if(altclockOffset_tuple != NULL) {
    globalSettings.altclockOffset = altclockOffset_tuple->value->int8;
  }

  if(activateDisconnectIcon_tuple != NULL) {
    globalSettings.activateDisconnectIcon = (bool)activateDisconnectIcon_tuple->value->int8;
  }

  Settings_updateDynamicSettings();

  // save the new settings to persistent storage
  Settings_saveToStorage();

  // notify the main screen, in case something changed
  message_processed_callback();
}

//static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
//}

//static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  // APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! %d %d %d", reason, APP_MSG_SEND_TIMEOUT, APP_MSG_SEND_REJECTED);

//}

//static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  // APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
//}

void messaging_requestNewWeatherData(void) {
  // just send an empty message for now
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint32(iter, 0, 0);
  app_message_outbox_send();
}

void messaging_init(MessageProcessedCallback processed_callback) {
  // register my custom callback
  message_processed_callback = processed_callback;

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  //app_message_register_inbox_dropped(inbox_dropped_callback);
  //app_message_register_outbox_failed(outbox_failed_callback);
  //app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(512, 8);

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "Watch messaging is started!");
}
