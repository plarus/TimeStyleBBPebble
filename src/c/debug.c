#include <pebble.h>
#include "debug.h"

//static int16_t Debug_displayUpdate = 0;
/*int Debug_sidebarUpdate = 0;
int Debug_clockAreaUpdate = 0;

int16_t Debug_RedrawFunction = 0;
int Debug_healthSleepCall = 0;
int Debug_restfulSleeping = 0;
int16_t Debug_focusChange = 0;
int16_t Debug_bluetoothStateChange = 0;
int16_t Debug_unobstructedAreaChange = 0;
*/

void Debug_display(void) {  
// Generic log
  APP_LOG(APP_LOG_LEVEL_DEBUG,"\n******");
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Avail RAM: %d", heap_bytes_free());
 /* Debug_displayUpdate++;
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Nb disp: %d", Debug_displayUpdate);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Sidebar refr: %d", Debug_sidebarUpdate);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Clck area refr: %d", Debug_clockAreaUpdate);
*/
  // Specific log
/*
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw call: %d", Debug_RedrawFunction);

  APP_LOG(APP_LOG_LEVEL_DEBUG,"Sleep upd: %d", Debug_healthSleepCall);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Restful sleep: %d", Debug_restfulSleeping);

  APP_LOG(APP_LOG_LEVEL_DEBUG,"Focus chg: %d", Debug_focusChange);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Bl chg: %d", Debug_bluetoothStateChange);
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Unobst Area chg: %d", Debug_unobstructedAreaChange);
*/
}
