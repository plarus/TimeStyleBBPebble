#include <pebble.h>
#include <math.h>
#include "settings.h"
#include "util.h"

bool recolor_iterator_cb(GDrawCommand *command, uint32_t index, void *context) {
  GColor *colors = (GColor *)context;

  gdraw_command_set_fill_color(command, colors[0]);
  gdraw_command_set_stroke_color(command, colors[1]);

  return true;
}

/*
 * For the specified GDrawCommandImage, recolors it with
 * the specified fill and stroke colors
 */
void image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color) {
  GColor colors[2];
  colors[0] = fill_color;
  colors[1] = stroke_color;

  gdraw_command_list_iterate(gdraw_command_image_get_command_list(img),
                             recolor_iterator_cb, &colors);
}

void util_image_draw(GContext* ctx, GDrawCommandImage *img, int xPosition, int yPosition) {
  image_recolor(img, globalSettings.iconFillColor, globalSettings.iconStrokeColor);
  gdraw_command_image_draw(ctx, img, GPoint(xPosition, yPosition));
}

void util_image_draw_inverted_color(GContext* ctx, GDrawCommandImage *img, int xPosition, int yPosition) {
  image_recolor(img, globalSettings.iconStrokeColor, globalSettings.iconFillColor);
  gdraw_command_image_draw(ctx, img, GPoint(xPosition, yPosition));
}

void seconds_to_minutes_hours_text(HealthValue seconds, char * hours_text, char * minutes_text) {

    // convert to hours/minutes
    int minutes = seconds / 60;
    int hours   = minutes / 60;

    // find minutes remainder
    minutes %= 60;

    snprintf(hours_text, sizeof(hours_text), "%ih", hours);
    snprintf(minutes_text, sizeof(minutes_text), "%im", minutes);
}

void seconds_to_text(HealthValue seconds, char * hours_minutes_text) {

     // Check step data is available
    int minutes = seconds / 60;
    int hours   = minutes / 60;

    // find minutes remainder
    minutes %= 60;

    snprintf(hours_minutes_text, sizeof(hours_minutes_text), "%ih%i", hours, minutes);
   }

void distance_to_metric_text(HealthValue distance, char * metric_text) {
    if(distance < 100) {
      snprintf(metric_text, sizeof(metric_text), "%lim", distance);
    } else if(distance < 1000) {
      distance /= 100; // convert to tenths of km
      snprintf(metric_text, sizeof(metric_text), "%c%likm", globalSettings.decimalSeparator, distance);
    } else {
      distance /= 1000; // convert to km
      snprintf(metric_text, sizeof(metric_text), "%likm", distance);
    }
}

void distance_to_imperial_text(HealthValue distance, char * imperial_text) {
    int miles_tenths = distance * 10 / 1609 % 10;
    int miles_whole  = (int)roundf(distance / 1609.0f);

    if(miles_whole > 0) {
      snprintf(imperial_text, sizeof(imperial_text), "%imi", miles_whole);
    } else {
      snprintf(imperial_text, sizeof(imperial_text), "%c%imi", globalSettings.decimalSeparator, miles_tenths);
    }
}

void steps_to_text(HealthValue steps, char * steps_text) {

    if(steps < 1000) {
      snprintf(steps_text, sizeof(steps_text), "%li", steps);
    } else {
      int steps_thousands = steps / 1000;
      int steps_hundreds  = steps / 100 % 10;

      if (steps < 10000) {
        snprintf(steps_text, sizeof(steps_text), "%i%c%ik", steps_thousands, globalSettings.decimalSeparator, steps_hundreds);
      } else {
        snprintf(steps_text, sizeof(steps_text), "%ik", steps_thousands);
      }
   }
}

void kCalories_to_text(HealthValue kcalories, char * kcalories_text) {
    // format kcalories string
    if(kcalories < 1000) {
      snprintf(kcalories_text, sizeof(kcalories_text), "%likc", kcalories);
    } else {
      int kcalories_thousands = kcalories / 1000;
      int kcalories_hundreds  = kcalories / 100 % 10;

      if (kcalories < 10000) {
        snprintf(kcalories_text, sizeof(kcalories_text), "%i%c%iMc", kcalories_thousands, globalSettings.decimalSeparator, kcalories_hundreds);
      } else {
        snprintf(kcalories_text, sizeof(kcalories_text), "%iMc", kcalories_thousands);
      }
    }
}
