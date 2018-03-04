#pragma once

#include <pebble.h>
#include "fctx.h"

void ClockDigit_draw_string(FContext* fctx, const char* text, uint8_t fontId, int16_t pixels, GTextAlignment alignment, FTextAnchor anchor);
