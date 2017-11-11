#pragma once
#include <pebble.h>

/*
 * Draw image at position with the specified fill and stroke colors
 */
void util_image_draw(GContext* ctx, GDrawCommandImage *img, int xPosition, int yPosition);

/*
 * Draw image at position with the inverted fill and stroke colors
 */
void util_image_draw_inverted_color(GContext* ctx, GDrawCommandImage *img, int xPosition, int yPosition);

/*
 * Convert number of seconds to minutes and hours text
 */
void seconds_to_minutes_hours_text(HealthValue seconds, char * hours_text, char * minutes_text);

/*
 * Convert number of seconds to one minutes and hours text
 */
void seconds_to_text(HealthValue seconds, char * hours_minutes_text);
