#pragma once
#include <pebble.h>

// "public" functions
void Sidebar_init(Window* window);
void Sidebar_deinit(void);
void Sidebar_set_layer(void);
void Sidebar_redraw(void);
#ifndef PBL_ROUND
void Sidebar_set_hidden(bool hide);
#endif
