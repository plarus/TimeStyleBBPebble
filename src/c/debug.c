#include <pebble.h>
#include "debug.h"

static int16_t Debug_displayUpdate = 0;
int Debug_sidebarUpdate = 0;
int Debug_clockAreaUpdate = 0;

int16_t Debug_RedrawFunction = 0;
int Debug_healthEventHandlerCall = 0;
int16_t Debug_healthSignificantUpdate = 0;
int Debug_healthSleepCall = 0;
int16_t Debug_movSleepUpdate = 0;
int Debug_restfulSleeping = 0;
int Debug_walking = 0;
int Debug_running = 0;
int16_t Debug_batteryHandler = 0;
int8_t Debug_chargePercent = 0;
int16_t Debug_focusChange = 0;
int16_t Debug_bluetoothStateChange = 0;
int16_t Debug_unobstructedAreaChange = 0;

void Debug_display(void) {
  // Generic log
  APP_LOG(APP_LOG_LEVEL_DEBUG,"\n************************************");
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Currently available RAM: %d", heap_bytes_free());
  Debug_displayUpdate++;
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Nb display: %d", Debug_displayUpdate);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Sidebar refresh: %d", Debug_sidebarUpdate);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Clock area refresh: %d", Debug_clockAreaUpdate);

  // Specific log
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw call: %d", Debug_RedrawFunction);

  APP_LOG(APP_LOG_LEVEL_DEBUG,"Health handler call: %d", Debug_healthEventHandlerCall);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Health significant call: %d", Debug_healthSignificantUpdate);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Sleep update: %d", Debug_healthSleepCall);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Restful sleeping: %d", Debug_restfulSleeping);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Walking: %d", Debug_walking);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Running: %d", Debug_running);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Sleep update on movement: %d", Debug_movSleepUpdate);

  APP_LOG(APP_LOG_LEVEL_DEBUG,"Battery handler: %d", Debug_batteryHandler);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Battery: %d", Debug_chargePercent);

  APP_LOG(APP_LOG_LEVEL_DEBUG,"Focus change: %d", Debug_focusChange);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Bluetooth change: %d", Debug_bluetoothStateChange);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Unobstructed Area change: %d", Debug_unobstructedAreaChange);

}
