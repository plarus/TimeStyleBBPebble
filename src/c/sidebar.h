#pragma once
#include <pebble.h>

// "public" functions
void Sidebar_init(Window* window);
void Sidebar_deinit(void);
void Sidebar_redraw(void);
void Sidebar_updateTime(struct tm* timeInfo);
