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
