#include <pebble.h>
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>
#include "clock_area.h"
#include "settings.h"
#include "languages.h"
#include "time_date.h"

#define ROUND_VERTICAL_PADDING 15

static Layer* clock_area_layer;
static FFont* hours_font;
static FFont* minutes_font;
static FFont* colon_font;

static GFont date_font;
static GFont am_pm_font;

// just allocate all the fonts at startup because i don't feel like
// dealing with allocating and deallocating things
static FFont* avenir;
static FFont* avenir_bold;
static FFont* leco;

static GRect screen_rect;

// "private" functions
static void update_fonts(void) {
  switch(globalSettings.clockFontId) {
    case FONT_SETTING_DEFAULT:
        hours_font = avenir;
        minutes_font = avenir;
        colon_font = avenir;
      break;
    case FONT_SETTING_BOLD:
        hours_font = avenir_bold;
        minutes_font = avenir_bold;
        colon_font = avenir_bold;
      break;
    case FONT_SETTING_BOLD_H:
        hours_font = avenir_bold;
        minutes_font = avenir;
        colon_font = avenir;
      break;
    case FONT_SETTING_BOLD_M:
        hours_font = avenir;
        minutes_font = avenir_bold;
        colon_font = avenir;
      break;
    case FONT_SETTING_LECO:
        hours_font = leco;
        minutes_font = leco;
        colon_font = leco;
      break;
  }
}

static void update_original_clock_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  GRect bounds;

  #ifndef PBL_ROUND
    bounds = layer_get_unobstructed_bounds(l);

    if (globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP){
      bounds.size.h -= FIXED_WIDGET_HEIGHT;
    }
  #else
    bounds = GRect(0, ROUND_VERTICAL_PADDING, screen_rect.size.w, screen_rect.size.h - ROUND_VERTICAL_PADDING * 2);
  #endif

  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, globalSettings.timeColor);


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
    #endif
  } else {
    #ifdef PBL_COLOR
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
    }else if(globalSettings.sidebarLocation == TOP) {
      v_adjust += FIXED_WIDGET_HEIGHT;
    }
  #endif

  FPoint time_pos;
  fctx_begin_fill(&fctx);
  fctx_set_text_em_height(&fctx, hours_font, font_size);
  fctx_set_text_em_height(&fctx, minutes_font, font_size);

  // draw hours
  time_pos.x = INT_TO_FIXED(bounds.size.w / 2 + h_adjust);
  time_pos.y = INT_TO_FIXED(v_padding + v_adjust);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_date_hours, hours_font, GTextAlignmentCenter, FTextAnchorTop);

  //draw minutes
  time_pos.y = INT_TO_FIXED(bounds.size.h - v_padding + v_adjust);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_date_minutes, minutes_font, GTextAlignmentCenter, FTextAnchorBaseline);

  fctx_end_fill(&fctx);
  fctx_deinit_context(&fctx);
}

static void update_clock_and_date_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  GRect fullscreen_bounds = layer_get_bounds(l);
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(l);

  int16_t obstruction_height = fullscreen_bounds.size.h - unobstructed_bounds.size.h;

  // initialize FCTX, the fancy 3rd party drawing library that all the cool kids use
  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, globalSettings.timeColor);

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
    #endif
  } else {
    #ifdef PBL_COLOR
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
  fctx_begin_fill(&fctx);
  fctx_set_text_em_height(&fctx, hours_font, font_size);
  fctx_set_text_em_height(&fctx, minutes_font, font_size);
  fctx_set_text_em_height(&fctx, colon_font, font_size);

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
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_date_hours, hours_font, GTextAlignmentRight, FTextAnchorTop);

  //draw ":"
  if(globalSettings.clockFontId == FONT_SETTING_LECO) {
    time_pos.x = INT_TO_FIXED(h_middle);
  } else {
    time_pos.x = INT_TO_FIXED(h_middle - 1);
  }
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, ":", colon_font, GTextAlignmentCenter, FTextAnchorTop);

  //draw minutes
  time_pos.x = INT_TO_FIXED(h_middle + h_colon_margin + h_adjust);
  fctx_set_offset(&fctx, time_pos);
  fctx_draw_string(&fctx, time_date_minutes, minutes_font, GTextAlignmentLeft, FTextAnchorTop);

  // draw date
  graphics_draw_text(ctx,
                     time_date_currentDate,
                     date_font,
                     GRect(0, fullscreen_bounds.size.h / 2 - 11 + v_adjust, fullscreen_bounds.size.w, 30),
                     GTextOverflowModeFill,
                     GTextAlignmentCenter,
                     NULL);

  fctx_end_fill(&fctx);
  fctx_deinit_context(&fctx);
}

static void update_clock_area_layer(Layer *l, GContext* ctx) {

  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
      update_clock_and_date_area_layer(l, ctx);
  } else {
      update_original_clock_area_layer(l, ctx);
  }
}

void ClockArea_init(Window* window) {
  // record the screen size, since we NEVER GET IT AGAIN
  screen_rect = layer_get_bounds(window_get_root_layer(window));

  GRect bounds;
  bounds = GRect(0, 0, screen_rect.size.w, screen_rect.size.h);

  // init the clock area layer
  clock_area_layer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), clock_area_layer);
  layer_set_update_proc(clock_area_layer, update_clock_area_layer);

  // allocate fonts
  avenir =      ffont_create_from_resource(RESOURCE_ID_AVENIR_REGULAR_FFONT);
  avenir_bold = ffont_create_from_resource(RESOURCE_ID_AVENIR_BOLD_FFONT);
  leco =        ffont_create_from_resource(RESOURCE_ID_LECO_REGULAR_FFONT);

  date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  am_pm_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);

  // select fonts based on settings
  update_fonts();
}

void ClockArea_deinit(void) {
  layer_destroy(clock_area_layer);

  ffont_destroy(avenir);
  ffont_destroy(avenir_bold);
  ffont_destroy(leco);
}

void ClockArea_redraw(void) {
  // check if the fonts need to be switched
  update_fonts();

  layer_mark_dirty(clock_area_layer);
}
