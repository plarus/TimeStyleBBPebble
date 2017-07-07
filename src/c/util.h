#pragma once
#include <pebble.h>

/*
 * For the specified GDrawCommandImage, recolors it with
 * the specified fill and stroke colors
 */
void gdraw_command_image_recolor(GDrawCommandImage *img, GColor fill_color, GColor stroke_color);

/*
 * Get obstruction height of Timeline Quick View on the layer given as input
 */
int16_t get_obstruction_height(Layer *s_window_layer);

/*
 * Convert number of seconds to minutes and hours text
 */
void seconds_to_minutes_hours_text(HealthValue seconds, char * hours_text, char * minutes_text);

/*
 * Convert number of seconds to one minutes and hours text
 */
void seconds_to_text(HealthValue seconds, char * hours_minutes_text);

/*
 * Convert distance to metric text
 */
void distance_to_metric_text(HealthValue distance, char * metric_text);

/*
 * Convert distance to imperial unit text
 */
void distance_to_imperial_text(HealthValue distance, char * imperial_text);

/*
 * Convert steps to text
 */
void steps_to_text(HealthValue steps, char * steps_text);

/*
 * Convert kCalories to text
 */
void kCalories_to_text(kcalories, kcalories_text);
