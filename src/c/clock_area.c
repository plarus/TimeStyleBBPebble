#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#include "clock_area.h"
#include "settings.h"
#include "languages.h"
#include "time_date.h"
#include "debug.h"

#define ROUND_VERTICAL_PADDING 15

static Layer* clock_area_layer;

// just allocate all the fonts at startup because i don't feel like
// dealing with allocating and deallocating things
static FFont* hours_font;
static FFont* minutes_font;
static FFont* colon_font;

#ifndef PBL_ROUND
static GFont date_font;
static GFont am_pm_font;
#else
static GRect screen_rect;
#endif

// "private" functions
static void update_original_clock_area_layer(Layer *l, GContext* ctx, FContext* fctx) {
  // check layer bounds
  GRect bounds;

  #ifndef PBL_ROUND
    bounds = layer_get_unobstructed_bounds(l);
  #else
    bounds = GRect(0, ROUND_VERTICAL_PADDING, screen_rect.size.w, screen_rect.size.h - ROUND_VERTICAL_PADDING * 2);
  #endif

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

    // leco looks awful with antialiasing
    #ifdef PBL_COLOR
      fctx_enable_aa(false);
  } else {
      fctx_enable_aa(true);
    #endif
  }

  // if it's a round watch, EVERYTHING CHANGES
  #ifdef PBL_ROUND
    v_adjust = ROUND_VERTICAL_PADDING;

    if(globalSettings.clockFontId != FONT_SETTING_LECO) {
      h_adjust = -1;
    }
  #else
    // for rectangular watches, adjust X position based on sidebar position
    if(globalSettings.sidebarLocation == RIGHT) {
      h_adjust -= ACTION_BAR_WIDTH / 2 + 1;
    } else if(globalSettings.sidebarLocation == LEFT) {
      h_adjust += ACTION_BAR_WIDTH / 2;
    }
  #endif

  FPoint time_pos;

  // draw hours
  time_pos.x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);
  time_pos.y = INT_TO_FIXED(v_padding + v_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, hours_font, font_size);
  fctx_draw_string(fctx, time_date_hours, hours_font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(fctx);

  //draw minutes
  time_pos.y = INT_TO_FIXED(bounds.size.h - v_padding + v_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, minutes_font, font_size);
  fctx_draw_string(fctx, time_date_minutes, minutes_font, GTextAlignmentCenter, FTextAnchorBaseline);
  fctx_end_fill(fctx);
}

#ifndef PBL_ROUND
static void update_clock_and_date_area_layer(Layer *l, GContext* ctx, FContext* fctx) {
  // check layer bounds
  GRect fullscreen_bounds = layer_get_bounds(l);
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(l);

  int16_t obstruction_height = fullscreen_bounds.size.h - unobstructed_bounds.size.h;

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
    v_adjust = 0;

    // leco looks awful with antialiasing
    #ifdef PBL_COLOR
      fctx_enable_aa(false);
  } else {
      fctx_enable_aa(true);
    #endif
  }

  // for rectangular watches, adjust X position based on sidebar position
  if(globalSettings.sidebarLocation == BOTTOM) {
    v_adjust -= 3;
  } else {
    v_adjust += FIXED_WIDGET_HEIGHT - obstruction_height - 3;
  }

  int h_middle = fullscreen_bounds.size.w / 2;
  int h_colon_margin = 7;

  FPoint time_pos;

  graphics_context_set_text_color(ctx, globalSettings.timeColor);

  if(!clock_is_24h_style()) {
    // draw am/pm
    graphics_draw_text(ctx,
                       time_date_isAmHour ? "AM" : "PM",
                       am_pm_font,
                       GRect(0, v_padding / 2 + v_adjust, fullscreen_bounds.size.w - h_colon_margin + h_adjust, 20),
                       GTextOverflowModeFill,
                       GTextAlignmentRight,
                       NULL);
  }

  // draw hours
  time_pos.x = INT_TO_FIXED(h_middle - h_colon_margin + h_adjust);
  time_pos.y = INT_TO_FIXED(3 * v_padding + v_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, hours_font, font_size);
  fctx_draw_string(fctx, time_date_hours, hours_font, GTextAlignmentRight, FTextAnchorTop);
  fctx_end_fill(fctx);

  //draw ":"
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    time_pos.x = INT_TO_FIXED(h_middle);
  } else {
    time_pos.x = INT_TO_FIXED(h_middle - 1);
  }
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, colon_font, font_size);
  fctx_draw_string(fctx, ":", colon_font, GTextAlignmentCenter, FTextAnchorTop);
  fctx_end_fill(fctx);

  //draw minutes
  time_pos.x = INT_TO_FIXED(h_middle + h_colon_margin + h_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, minutes_font, font_size);
  fctx_draw_string(fctx, time_date_minutes, minutes_font, GTextAlignmentLeft, FTextAnchorTop);
  fctx_end_fill(fctx);

  // draw date
  graphics_draw_text(ctx,
                     time_date_currentDate,
                     date_font,
                     GRect(0, fullscreen_bounds.size.h / 2 - 11 + v_adjust, fullscreen_bounds.size.w, 30),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);
}

#else

static void update_one_line_clock_area_layer(Layer *l, GContext* ctx, FContext* fctx) {
  // check layer bounds
  GRect fullscreen_bounds = layer_get_bounds(l);

  // calculate font size
  int font_size = fullscreen_bounds.size.h / 3 + 7;

  // avenir + avenir bold metrics
  int h_adjust = -2;

  // alternate metrics for LECO
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    h_adjust = -3;

    // leco looks awful with antialiasing
    #ifdef PBL_COLOR
      fctx_enable_aa(false);
  } else {
      fctx_enable_aa(true);
    #endif
  }

  int h_middle = fullscreen_bounds.size.w / 2;
  int h_colon_margin = 7;

  FPoint time_pos;

  // draw hours
  time_pos.x = INT_TO_FIXED(h_middle - h_colon_margin + h_adjust);
  time_pos.y = INT_TO_FIXED(fullscreen_bounds.size.h / 2);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, hours_font, font_size);
  fctx_draw_string(fctx, time_date_hours, hours_font, GTextAlignmentRight, FTextAnchorMiddle);
  fctx_end_fill(fctx);

  //draw ":"
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    time_pos.x = INT_TO_FIXED(h_middle);
  } else {
    time_pos.x = INT_TO_FIXED(h_middle - 1);
  }
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, colon_font, font_size);
  fctx_draw_string(fctx, ":", colon_font, GTextAlignmentCenter, FTextAnchorMiddle);
  fctx_end_fill(fctx);

  //draw minutes
  time_pos.x = INT_TO_FIXED(h_middle + h_colon_margin + h_adjust);
  fctx_begin_fill(fctx);
  fctx_set_offset(fctx, time_pos);
  fctx_set_text_em_height(fctx, minutes_font, font_size);
  fctx_draw_string(fctx, time_date_minutes, minutes_font, GTextAlignmentLeft, FTextAnchorMiddle);
  fctx_end_fill(fctx);
}
#endif

static void update_clock_area_layer(Layer *l, GContext* ctx) {
  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, globalSettings.timeColor);

  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
#ifdef PBL_ROUND
      update_one_line_clock_area_layer(l, ctx, &fctx);
#else
      update_clock_and_date_area_layer(l, ctx, &fctx);
#endif // PBL_ROUND
  } else {
      update_original_clock_area_layer(l, ctx, &fctx);
  }

  fctx_deinit_context(&fctx);

  /* Debug */ //Debug_clockAreaUpdate++;
}

void ClockArea_init(Window* window) {
#ifndef PBL_ROUND
  GRect screen_rect;
#endif

  // record the screen size, since we NEVER GET IT AGAIN
  screen_rect = layer_get_bounds(window_get_root_layer(window));

  GRect bounds;
  bounds = GRect(0, 0, screen_rect.size.w, screen_rect.size.h);

  // init the clock area layer
  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);
}

void ClockArea_ffont_destroy(void) {
  switch(globalSettings.prev_clockFontId) {
    case FONT_SETTING_DEFAULT:
    case FONT_SETTING_BOLD:
    case FONT_SETTING_LECO:
        ffont_destroy(hours_font);
      break;
    case FONT_SETTING_BOLD_H:
    case FONT_SETTING_BOLD_M:
        ffont_destroy(hours_font);
        ffont_destroy(minutes_font);
      break;
  }
}

void ClockArea_deinit(void) {
  layer_destroy(clock_area_layer);

  ClockArea_ffont_destroy();
}

void ClockArea_redraw(void) {
  layer_mark_dirty(clock_area_layer);
}

void ClockArea_update_fonts(void) {
#ifndef PBL_ROUND
  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
    date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    if(!clock_is_24h_style()) {
      am_pm_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    }
  }
#endif

  if(globalSettings.prev_clockFontId != globalSettings.clockFontId) {
    if(globalSettings.prev_clockFontId != FONT_SETTING_UNSET) {
      ClockArea_ffont_destroy();
    }

    FFont* avenir;
    FFont* avenir_bold;
    FFont* leco;

    switch(globalSettings.clockFontId) {
      case FONT_SETTING_DEFAULT:
          avenir = ffont_create_from_resource(RESOURCE_ID_AVENIR_REGULAR_FFONT);

          hours_font = avenir;
          minutes_font = avenir;
          colon_font = avenir;
        break;
      case FONT_SETTING_BOLD:
          avenir_bold = ffont_create_from_resource(RESOURCE_ID_AVENIR_BOLD_FFONT);

          hours_font = avenir_bold;
          minutes_font = avenir_bold;
          colon_font = avenir_bold;
        break;
      case FONT_SETTING_BOLD_H:
          avenir =      ffont_create_from_resource(RESOURCE_ID_AVENIR_REGULAR_FFONT);
          avenir_bold = ffont_create_from_resource(RESOURCE_ID_AVENIR_BOLD_FFONT);

          hours_font = avenir_bold;
          minutes_font = avenir;
          colon_font = avenir;
        break;
      case FONT_SETTING_BOLD_M:
          avenir =      ffont_create_from_resource(RESOURCE_ID_AVENIR_REGULAR_FFONT);
          avenir_bold = ffont_create_from_resource(RESOURCE_ID_AVENIR_BOLD_FFONT);

          hours_font = avenir;
          minutes_font = avenir_bold;
          colon_font = avenir;
        break;
      case FONT_SETTING_LECO:
          leco = ffont_create_from_resource(RESOURCE_ID_LECO_REGULAR_FFONT);

          hours_font = leco;
          minutes_font = leco;
          colon_font = leco;
        break;
    }
    globalSettings.prev_clockFontId = globalSettings.clockFontId;
  }
}
