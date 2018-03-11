#include <pebble.h>
#include "fctx.h"
#include "ffont.h"
#include "clock_area.h"
#include "clock_digit.h"
#include "settings.h"
#include "time_date.h"

static Layer* clock_area_layer;

static uint8_t hours_font;
static uint8_t minutes_font;
static uint8_t colon_font;

static GFont date_font;

// "private" functions
static void update_original_clock_area_layer(Layer *l, GContext* ctx, FContext* fctx) {
  // check layer bounds
  GRect bounds = layer_get_unobstructed_bounds(l);

  // calculate font size
  int font_size = 4 * bounds.size.h / 7;

  // avenir + avenir bold metrics
  int v_padding = bounds.size.h / 16;
  int h_adjust = 0;
  int v_adjust = 0;

  // alternate metrics for LECO
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    font_size += 6;
    v_padding = bounds.size.h / 20;
    h_adjust = -4;
    v_adjust = 0;
  }

  // for rectangular watches, adjust X position based on sidebar position
  if(globalSettings.sidebarLocation == RIGHT) {
    h_adjust -= ACTION_BAR_WIDTH / 2 + 1;
  } else if(globalSettings.sidebarLocation == LEFT) {
    h_adjust += ACTION_BAR_WIDTH / 2;
  }

  FPoint time_pos;

  // draw hours
  time_pos.x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);
  time_pos.y = INT_TO_FIXED(v_padding + v_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  ClockDigit_draw_string(fctx, time_date_hours, hours_font, font_size, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(fctx);

  //draw minutes
  time_pos.y = INT_TO_FIXED(bounds.size.h - v_padding + v_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  ClockDigit_draw_string(fctx, time_date_minutes, minutes_font, font_size, GTextAlignmentCenter, FTextAnchorBaseline);
  fctx_end_fill(fctx);
}

static void update_clock_and_date_area_layer(Layer *l, GContext* ctx, FContext* fctx) {
  // check layer bounds
  GRect fullscreen_bounds = layer_get_bounds(l);

  // calculate font size
  int font_size = fullscreen_bounds.size.h / 3;

  // avenir + avenir bold metrics
  int v_padding = fullscreen_bounds.size.h / 16;
  int h_adjust = -2;
  int v_adjust = 0;

  // alternate metrics for LECO
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    v_padding = fullscreen_bounds.size.h / 20;
    h_adjust = -3;
  }

  // for rectangular watches, adjust X position based on sidebar position
  if(globalSettings.sidebarLocation == TOP) {
    v_adjust += FIXED_WIDGET_HEIGHT;
  }

  int h_middle = fullscreen_bounds.size.w / 2;
  int h_colon_margin = 7;

  FPoint time_pos;

  if(globalSettings.centerTime == false || globalSettings.clockFontId == FONT_SETTING_BOLD_H || globalSettings.clockFontId == FONT_SETTING_BOLD_M) {
    // draw hours
    time_pos.x = INT_TO_FIXED(h_middle - h_colon_margin + h_adjust);
    time_pos.y = INT_TO_FIXED(3 * v_padding + v_adjust);
    fctx_begin_fill(fctx);
    fctx_set_offset(fctx, time_pos);
    ClockDigit_draw_string(fctx, time_date_hours, hours_font, font_size, GTextAlignmentRight, FTextAnchorTop);
    fctx_end_fill(fctx);

    //draw ":"
    time_pos.x = INT_TO_FIXED(h_middle - 1);
    fctx_begin_fill(fctx);
    fctx_set_offset(fctx, time_pos);
    ClockDigit_draw_string(fctx, ":", colon_font, font_size, GTextAlignmentCenter, FTextAnchorTop);
    fctx_end_fill(fctx);

    //draw minutes
    time_pos.x = INT_TO_FIXED(h_middle + h_colon_margin + h_adjust);
    fctx_begin_fill(fctx);
    fctx_set_offset(fctx, time_pos);
    ClockDigit_draw_string(fctx, time_date_minutes, minutes_font, font_size, GTextAlignmentLeft, FTextAnchorTop);
    fctx_end_fill(fctx);
  } else {
    // if only one font center all
    char time[6];

    strncpy(time, time_date_hours, sizeof(time_date_hours));
    strncat(time, ":" , 2);
    strncat(time, time_date_minutes, sizeof(time_date_minutes));

    time_pos.x = INT_TO_FIXED(h_middle - 2);
    time_pos.y = INT_TO_FIXED(3 * v_padding + v_adjust);
    fctx_begin_fill(fctx);
    fctx_set_offset(fctx, time_pos);
    ClockDigit_draw_string(fctx, time, colon_font, font_size, GTextAlignmentCenter, FTextAnchorTop);
    fctx_end_fill(fctx);
  }

  char time_date_currentDate[21];

  strncpy(time_date_currentDate, globalSettings.languageDayNames[time_date_currentDayName], sizeof(globalSettings.languageDayNames[time_date_currentDayName]));
  strncat(time_date_currentDate, " " , 2);
  strncat(time_date_currentDate, time_date_currentDayNum, sizeof(time_date_currentDayNum));
  strncat(time_date_currentDate, " " , 2);
  strncat(time_date_currentDate, globalSettings.languageMonthNames[time_date_currentMonth], sizeof(globalSettings.languageMonthNames[time_date_currentMonth]));

  // draw date
  graphics_context_set_text_color(ctx, globalSettings.timeColor);
  graphics_draw_text(ctx,
                     time_date_currentDate,
                     date_font,
                     GRect(0, fullscreen_bounds.size.h / 2 - 11 + v_adjust, fullscreen_bounds.size.w, 30),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}


static void update_clock_area_layer(Layer *l, GContext* ctx) {
  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_fill_color(&fctx, globalSettings.timeColor);

  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
      update_clock_and_date_area_layer(l, ctx, &fctx);
  } else {
      update_original_clock_area_layer(l, ctx, &fctx);
  }
  fctx_deinit_context(&fctx);
}

void ClockArea_init(Window* window) {
  // record the screen size, since we NEVER GET IT AGAIN
  GRect screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds = GRect(0, 0, screen_rect.size.w, screen_rect.size.h);

  // init the clock area layer
  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);
}

void ClockArea_deinit(void) {
  layer_destroy(clock_area_layer);
}

void ClockArea_redraw(void) {
  layer_mark_dirty(clock_area_layer);
}

void ClockArea_update_fonts(void) {
  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
    date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  }

  switch(globalSettings.clockFontId) {
    case FONT_SETTING_DEFAULT:
        hours_font = FONT_SETTING_DEFAULT;
        minutes_font = FONT_SETTING_DEFAULT;
        colon_font = FONT_SETTING_DEFAULT;
      break;
    case FONT_SETTING_BOLD:
        hours_font = FONT_SETTING_BOLD;
        minutes_font = FONT_SETTING_BOLD;
        colon_font = FONT_SETTING_BOLD;
      break;
    case FONT_SETTING_BOLD_H:
        hours_font = FONT_SETTING_BOLD;
        minutes_font = FONT_SETTING_DEFAULT;
        colon_font = FONT_SETTING_DEFAULT;
      break;
    case FONT_SETTING_BOLD_M:
        hours_font = FONT_SETTING_DEFAULT;
        minutes_font = FONT_SETTING_BOLD;
        colon_font = FONT_SETTING_DEFAULT;
      break;
    case FONT_SETTING_LECO:
        hours_font = FONT_SETTING_LECO;
        minutes_font = FONT_SETTING_LECO;
        colon_font = FONT_SETTING_LECO;
      break;
  }
}
