
var weather = require('./weather');
var languages = require('./languages');

// Require the keys' numeric values.
var keys = require('message_keys');

const CONFIG_VERSION = 11;
// const BASE_CONFIG_URL = 'http://localhost:4000/';
const BASE_CONFIG_URL = 'http://freakified.github.io/TimeStylePebble/';

const BarPosition = {
  NONE     : 0,
  LEFT     : 1,
  RIGHT    : 2,
  BOTTOM   : 3,
  TOP      : 4
}

const WidgetType = {
  EMPTY                     : 0,
  BLUETOOTH_DISCONNECT      : 1,
  BATTERY_METER             : 2,
  ALT_TIME_ZONE             : 3,
  DATE                      : 4,
  SECONDS                   : 5,
  WEEK_NUMBER               : 6,
  WEATHER_CURRENT           : 7,
  WEATHER_FORECAST_TODAY    : 8,
  TIME_UNUSED               : 9,
  HEALTH                    : 10,
  BEATS                     : 11,
  HEARTRATE                 : 12,
  SLEEP                     : 13,
  STEP                      : 14
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('JS component is now READY');

    // if it has never been started, set the weather to disabled
    // this is because the weather defaults to "off"
    if(window.localStorage.getItem('disable_weather') === null) {
      window.localStorage.setItem('disable_weather', 'yes');
    }

    console.log('the wdisabled value is: "' + window.localStorage.getItem('disable_weather') + '"');
    // if applicable, get the weather data
    if(window.localStorage.getItem('disable_weather') != 'yes') {
      weather.updateWeather();
    }
  }
);

// Listen for incoming messages
// when one is received, we simply assume that it is a request for new weather data
Pebble.addEventListener('appmessage',
  function(msg) {
    console.log('Received message: ' + JSON.stringify(msg.payload));

    // in the case of receiving this, we assume the watch does, in fact, need weather data
    window.localStorage.setItem('disable_weather', 'no');
    weather.updateWeather();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  const bwConfigURL      = BASE_CONFIG_URL + 'config_bw.html';
  const colorConfigURL   = BASE_CONFIG_URL + 'config_color.html';
  const roundConfigURL   = BASE_CONFIG_URL + 'config_color_round.html';
  const dioriteConfigURL = BASE_CONFIG_URL + 'config_diorite.html';

  const versionString = '?appversion=' + CONFIG_VERSION;

  if(Pebble.getActiveWatchInfo) {
    try {
      watch = Pebble.getActiveWatchInfo();
    } catch(err) {
      watch = {
        platform: "basalt"
      };
    }
  } else {
    watch = {
      platform: "aplite"
    };
  }

  if(watch.platform == "aplite"){
    Pebble.openURL(bwConfigURL + versionString);
  } else if(watch.platform == "chalk") {
    Pebble.openURL(roundConfigURL + versionString);
  } else if(watch.platform == "diorite") {
    Pebble.openURL(dioriteConfigURL + versionString);
  } else {
    Pebble.openURL(colorConfigURL + versionString);
  }
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = decodeURIComponent(e.response);

  if(configData) {
    configData = JSON.parse(decodeURIComponent(e.response));

    console.log("Config data received!" + JSON.stringify(configData));

    // prepare a structure to hold everything we'll send to the watch
    var dict = {};

    // color settings
    if(configData.color_bg) {
      dict.SettingColorBG = parseInt(configData.color_bg, 16);
    }

    if(configData.color_sidebar) {
      dict.SettingColorSidebar = parseInt(configData.color_sidebar, 16);
    }

    if(configData.color_time) {
      dict.SettingColorTime = parseInt(configData.color_time, 16);
    }

    if(configData.sidebar_text_color) {
      dict.SettingSidebarTextColor = parseInt(configData.sidebar_text_color, 16);
    }

    // general options
    if(configData.language_id !== undefined) {
      dict.SettingLanguageID = configData.language_id;
    }

    if(configData.leading_zero_setting) {
      if(configData.leading_zero_setting == 'yes') {
        dict.SettingShowLeadingZero = 1;
      } else {
        dict.SettingShowLeadingZero = 0;
      }
    }

    if(configData.center_time_setting) {
      if(configData.center_time_setting == 'yes') {
        dict.SettingCenterTime = 1;
      } else {
        dict.SettingCenterTime = 0;
      }
    }

    if(configData.clock_font_setting) {
      if(configData.clock_font_setting == 'default') {
        dict.SettingClockFontId = 0;
      } else if(configData.clock_font_setting == 'leco') {
        dict.SettingClockFontId = 1;
      } else if(configData.clock_font_setting == 'bold') {
        dict.SettingClockFontId = 2;
      } else if(configData.clock_font_setting == 'bold-h') {
        dict.SettingClockFontId = 3;
      } else if(configData.clock_font_setting == 'bold-m') {
        dict.SettingClockFontId = 4;
      }
    }

    // sidebar settings
    dict.SettingWidget0ID = configData.widget_0_id;
    dict.SettingWidget1ID = configData.widget_1_id;
    dict.SettingWidget2ID = configData.widget_2_id;
    dict.SettingWidget3ID = configData.widget_3_id;

    if(configData.sidebar_position) {
      if(configData.sidebar_position == 'left') {
        dict.SettingSidebarPosition = BarPosition.LEFT;
        dict.SettingWidget3ID = WidgetType.EMPTY;
      } else if(configData.sidebar_position == 'right') {
        dict.SettingSidebarPosition = BarPosition.RIGHT;
        dict.SettingWidget3ID = WidgetType.EMPTY;
      } else if(configData.sidebar_position == 'bottom') {
        dict.SettingSidebarPosition = BarPosition.BOTTOM;
      } else if(configData.sidebar_position == 'top') {
        dict.SettingSidebarPosition = BarPosition.TOP;
      } else { // 'none'
        dict.SettingSidebarPosition = BarPosition.NONE;
        dict.SettingWidget0ID = WidgetType.EMPTY;
        dict.SettingWidget1ID = WidgetType.EMPTY;
        dict.SettingWidget2ID = WidgetType.EMPTY;
        dict.SettingWidget3ID = WidgetType.EMPTY;
      }
    }

    // 2 bar only for pebble round
    if(watch.platform == "chalk") {
      dict.SettingWidget1ID = WidgetType.EMPTY;
      dict.SettingWidget3ID = WidgetType.EMPTY;
    }

    // normalize widgets positions
    if(dict.SettingSidebarPosition == BarPosition.BOTTOM || dict.SettingSidebarPosition == BarPosition.TOP) {
      if(dict.SettingWidget1ID == WidgetType.EMPTY) {
        dict.SettingWidget1ID = dict.SettingWidget2ID;
        dict.SettingWidget2ID = WidgetType.EMPTY;
      }
      if(dict.SettingWidget2ID == WidgetType.EMPTY) {
        dict.SettingWidget2ID = dict.SettingWidget3ID;
        dict.SettingWidget3ID = WidgetType.EMPTY;
      }
    }

    var widgetIDs = [dict.SettingWidget0ID, dict.SettingWidget1ID, dict.SettingWidget2ID, dict.SettingWidget3ID];

    // notification settings
    if(configData.hourly_vibe_setting) {
      if(configData.hourly_vibe_setting == 'yes') {
        dict.SettingHourlyVibe = 1;
      } else if (configData.hourly_vibe_setting == 'half') {
        dict.SettingHourlyVibe = 2;
      } else {
        dict.SettingHourlyVibe = 0;
      }
    }

    // bluetooth vibe
    if(configData.bluetooth_vibe_setting) {
      if(configData.bluetooth_vibe_setting == 'yes') {
        dict.SettingBluetoothVibe = 1;
      } else {
        dict.SettingBluetoothVibe = 0;
      }
    }

    // sidebar options
    if(dict.SettingSidebarPosition != BarPosition.NONE) {
      // bluetooth disconnetion widget
      if(configData.disconnect_icon_setting) {
        if(configData.disconnect_icon_setting == 'yes') {
          dict.SettingDisconnectIcon = 1;
        } else {
          dict.SettingDisconnectIcon = 0;
        }
      }

      // large font for all widgets
      if(configData.use_large_sidebar_font_setting) {
        if(configData.use_large_sidebar_font_setting == 'yes') {
          dict.SettingUseLargeFonts = 1;
        } else {
          dict.SettingUseLargeFonts = 0;
        }
      }

      // replacable widget
      if(watch.platform == "chalk") {
        if(widgetIDs[0] == WidgetType.EMPTY) {
          dict.ReplacableWidget = 0;
        } else if(widgetIDs[2] == WidgetType.EMPTY) {
          dict.ReplacableWidget = 2;
        } else if(widgetIDs.indexOf(WidgetType.WEATHER_CURRENT) != -1) {
          dict.ReplacableWidget = widgetIDs.indexOf(WidgetType.WEATHER_CURRENT);
        } else if(widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY) != -1) {
          dict.ReplacableWidget = widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY);
        } else { // if we don't have any of those things, just replace the left widget
          dict.ReplacableWidget = 0;
        }
      } else {
        dict.ReplacableWidget = -1;
        for (i = 0; i < 3; i++) {
          if(widgetIDs[i] == WidgetType.EMPTY) {
            dict.ReplacableWidget = i;
          }
        }

        if(dict.ReplacableWidget == -1) {
          if(widgetIDs[3] == WidgetType.EMPTY && (dict.SettingSidebarPosition == BarPosition.BOTTOM || dict.SettingSidebarPosition == BarPosition.TOP)) {
            dict.ReplacableWidget = 3;
        
          // are there any bluetooth-enabled widgets? if so, they're the second-best candidates
          } else if(widgetIDs.indexOf(WidgetType.WEATHER_CURRENT) != -1) {
            dict.ReplacableWidget = widgetIDs.indexOf(WidgetType.WEATHER_CURRENT);
          } else if(widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY) != -1) {
            dict.ReplacableWidget = widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY);    
          } else { // if we don't have any of those things, just replace the middle widget
            dict.ReplacableWidget = 1;
          }
        }
      }

      // week number widget
      if(configData.language_id !== undefined && widgetIDs.indexOf(WidgetType.WEEK_NUMBER) != -1) {
        dict.SettingLanguageWordForWeek = languages.wordForWeek[configData.language_id];
      }

      // weather widget settings
      if(widgetIDs.indexOf(WidgetType.WEATHER_CURRENT) != -1 || widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY) != -1) {
        if(configData.units) {
          if(configData.units == 'c') {
            dict.SettingUseMetric = 1;
          } else {
            dict.SettingUseMetric = 0;
          }
        }

        // weather location/source configs are not the watch's concern
        if(configData.weather_loc !== undefined) {
          window.localStorage.setItem('weather_loc', configData.weather_loc);
          window.localStorage.setItem('weather_loc_lat', configData.weather_loc_lat);
          window.localStorage.setItem('weather_loc_lng', configData.weather_loc_lng);
        }

        if(configData.weather_datasource) {
          window.localStorage.setItem('weather_datasource', configData.weather_datasource);
          window.localStorage.setItem('weather_api_key', configData.weather_api_key);
        }
      }

      // battery widget settings
      if(configData.battery_meter_setting) {
        if(configData.battery_meter_setting == 'icon-and-percent') {
          dict.SettingShowBatteryPct = 1;
        } else if(configData.battery_meter_setting == 'icon-only') {
          dict.SettingShowBatteryPct = 0;
        }
      }

      if(configData.autobattery_setting) {
        // Autobattey widget can be displayed only when battery widget is not displayed
        if(configData.autobattery_setting == 'on' && widgetIDs.indexOf(WidgetType.BATTERY_METER) == -1) {
          dict.SettingDisableAutobattery = 0;
        } else {
          dict.SettingDisableAutobattery = 1;
        }
      }

      if(configData.altclock_name && widgetIDs.indexOf(WidgetType.ALT_TIME_ZONE) != -1) {
        dict.SettingAltClockName = configData.altclock_name;
      }

      if(configData.altclock_offset !== null && widgetIDs.indexOf(WidgetType.ALT_TIME_ZONE) != -1) {
        dict.SettingAltClockOffset = parseInt(configData.altclock_offset, 10);
      }

      if(watch.platform != "aplite"){
        if(configData.decimal_separator && (widgetIDs.indexOf(WidgetType.HEALTH) != -1 || widgetIDs.indexOf(WidgetType.STEP) != -1)) {
          dict.SettingDecimalSep = configData.decimal_separator;
        }

        // heath settings
        if(configData.health_activity_display && (widgetIDs.indexOf(WidgetType.HEALTH) != -1 || widgetIDs.indexOf(WidgetType.STEP) != -1)) {
          if(configData.health_activity_display == 'distance') {
            dict.SettingHealthActivityDisplay = 1;
          } else if(configData.health_activity_display == 'duration') {
            dict.SettingHealthActivityDisplay = 2;
          } else if(configData.health_activity_display == 'calories') {
            dict.SettingHealthActivityDisplay = 3;
          } else { // steps
            dict.SettingHealthActivityDisplay = 0;
          }
        }

        if(configData.health_use_restful_sleep && (widgetIDs.indexOf(WidgetType.HEALTH) != -1 || widgetIDs.indexOf(WidgetType.SLEEP) != -1)) {
          if(configData.health_use_restful_sleep == 'yes') {
            dict.SettingHealthUseRestfulSleep = 1;
          } else {
            dict.SettingHealthUseRestfulSleep = 0;
          }
        }
      }
    }

    // determine whether or not the weather checking should be enabled
    var disableWeather;

    // if there is either a current conditions or a today's forecast widget, enable the weather
    if(widgetIDs.indexOf(WidgetType.WEATHER_CURRENT) != -1 || widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY) != -1) {
        disableWeather = 'no';
    } else {
        disableWeather = 'yes';
    }

    window.localStorage.setItem('disable_weather', disableWeather);

    var enableForecast;

    if(widgetIDs.indexOf(WidgetType.WEATHER_FORECAST_TODAY) != -1) {
      enableForecast = 'yes';
    } else {
      enableForecast = 'no';
    }

    window.localStorage.setItem('enable_forecast', enableForecast);

    console.log('Preparing message: ', JSON.stringify(dict));

    // Send settings to Pebble watchapp
    Pebble.sendAppMessage(dict, function(){
      // Second part of data (send datas in 2 part in order to reduce buffer size on Pebble watch)
      if(configData.language_id !== undefined && (dict.SettingSidebarPosition > 2 || widgetIDs.indexOf(WidgetType.DATE) != -1)) {
        // reset the structure
        dict = {};

        for (i = 0; i < 7; i++) {
          dict[keys.SettingLanguageDayNames + i] = languages.dayNames[configData.language_id][i];
        }
        for (i = 0; i < 12; i++) {
          dict[keys.SettingLanguageMonthNames + i] = languages.monthNames[configData.language_id][i];
        }

        console.log('Preparing language message: ', JSON.stringify(dict));

        // Send language information to Pebble watchapp
        Pebble.sendAppMessage(dict, function(){
          console.log('Sent language data to Pebble, now trying to get weather');

          if(window.localStorage.getItem('disable_weather') != 'yes') {
            // after sending config data, force a weather refresh in case that changed
            weather.updateWeather(true);
          }
        }, function() {
            console.log('Failed to send language data!');
        });
      } else {
        console.log('Sent config data to Pebble, now trying to get weather');

        if(window.localStorage.getItem('disable_weather') != 'yes') {
          // after sending config data, force a weather refresh in case that changed
          weather.updateWeather(true);
        }
      }
    }, function() {
        console.log('Failed to send config data!');
    });
  } else {
    console.log("No settings changed!");
  }

});
