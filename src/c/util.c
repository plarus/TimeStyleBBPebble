#include <pebble.h>
#include "util.h"


bool recolor_iterator_cb(GDrawCommand *command, uint32_t index, void *context) {
  GColor *colors = (GColor *)context;

  gdraw_command_set_fill_color(command, colors[0]);
  gdraw_command_set_stroke_color(command, colors[1]);

  return true;
}

void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color) {
  GColor colors[2];
  colors[0] = fill_color;
  colors[1] = stroke_color;

  gdraw_command_list_iterate(gdraw_command_image_get_command_list(img),
                             recolor_iterator_cb, &colors);
}

int16_t get_obstruction_height(Layer *s_window_layer) {
    GRect fullscreen = layer_get_bounds(s_window_layer);
    GRect unobstructed_bounds = layer_get_unobstructed_bounds(s_window_layer);

    return fullscreen.size.h - unobstructed_bounds.size.h;
}

#ifdef PBL_HEALTH
   bool is_health_metric_accessible(HealthMetric metric) {
     time_t start = time_start_of_today();
     time_t end = time(NULL);

     // Check step data is available
     HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
     bool result = mask & HealthServiceAccessibilityMaskAvailable;

     return result;
   }

   bool is_user_sleeping() {
     // temporarily disable sleep function until Pebble gets their act together
     return false;
     //
    //  time_t now = time(NULL);
     //
    //  HealthActivityMask activities = HealthActivitySleep | HealthActivityRestfulSleep;
     //
    //  HealthServiceAccessibilityMask mask = health_service_any_activity_accessible(activities, now, now);
    //  bool sleep_access_available = mask & HealthServiceAccessibilityMaskAvailable;
     //
    //  if(sleep_access_available) {
    //    uint32_t current_activities = health_service_peek_current_activities();
    //    bool sleeping = current_activities & HealthActivitySleep || current_activities & HealthActivityRestfulSleep;
     //
    //    return sleeping;
    //  } else {
    //    return false;
    //  }
   }
 #endif
