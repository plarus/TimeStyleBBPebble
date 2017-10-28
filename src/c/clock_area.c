#include <pebble.h>
#include "clock_area.h"
#include "settings.h"
#include "languages.h"
#include "time_date.h"

static Layer* clock_area_layer;
// fonts
static GFont bitham;
static GFont bitham_bold;
static GFont leco;
static GFont date_font;

static GFont hours_font;
static GFont minutes_font;
static GFont colon_font;

static void update_clock_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  GRect fullscreen_bounds = layer_get_bounds(l);

  int x_pos;
  int y_pos;

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
  if(globalSettings.sidebarLocation == BOTTOM) {
    v_adjust -= 3;
  } else {
    v_adjust += FIXED_WIDGET_HEIGHT - 3;
  }

  int h_middle = fullscreen_bounds.size.w / 2;
  int h_colon_margin = 7;

  graphics_context_set_text_color(ctx, globalSettings.timeColor);

  // draw hours
  x_pos = h_middle - h_colon_margin + h_adjust;
  y_pos = 3 * v_padding + v_adjust;

  graphics_draw_text(ctx,
                     time_date_hours,
                     hours_font,
                     GRect(x_pos, y_pos, h_middle, 44),
                     GTextOverflowModeFill,
                     GTextAlignmentRight,
                     NULL);

  //draw ":"
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    x_pos = h_middle;
  } else {
    x_pos = h_middle - 1;
  }
  graphics_draw_text(ctx,
                     ":",
                     colon_font,
                     GRect(x_pos, y_pos, h_colon_margin + h_adjust, 44),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  //draw minutes
  x_pos = h_middle + h_colon_margin + h_adjust;
  graphics_draw_text(ctx,
                     time_date_minutes,
                     minutes_font,
                     GRect(x_pos, y_pos, fullscreen_bounds.size.w - x_pos, 44),
                     GTextOverflowModeFill,
                     GTextAlignmentLeft,
                     NULL);

  // draw date
  graphics_draw_text(ctx,
                     time_date_currentDate,
                     date_font,
                     GRect(0, fullscreen_bounds.size.h / 2 - 11 + v_adjust, fullscreen_bounds.size.w, 30),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

void ClockArea_init(Window* window) {
  // record the screen size, since we NEVER GET IT AGAIN
  GRect screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds = GRect(0, 0, screen_rect.size.w, screen_rect.size.h);

  // init the clock area layer
  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);

  // load fonts
  bitham =      fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
  bitham_bold = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  leco =        fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);

  date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
}

void ClockArea_deinit(void) {
  layer_destroy(clock_area_layer);
}


void ClockArea_redraw(void) {
  layer_mark_dirty(clock_area_layer);
}

void ClockArea_update_fonts(void) {
  switch(globalSettings.clockFontId) {
    case FONT_SETTING_DEFAULT:
        hours_font = bitham;
        minutes_font = bitham;
        colon_font = bitham;
      break;
    case FONT_SETTING_BOLD:
        hours_font = bitham_bold;
        minutes_font = bitham_bold;
        colon_font = bitham_bold;
      break;
    case FONT_SETTING_BOLD_H:
        hours_font = bitham_bold;
        minutes_font = bitham;
        colon_font = bitham;
      break;
    case FONT_SETTING_BOLD_M:
        hours_font = bitham;
        minutes_font = bitham_bold;
        colon_font = bitham;
      break;
    case FONT_SETTING_LECO:
        hours_font = leco;
        minutes_font = leco;
        colon_font = leco;
      break;
  }
}
