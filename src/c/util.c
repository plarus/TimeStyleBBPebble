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

