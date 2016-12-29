#include <pebble.h>
#include <ctype.h>
#include <math.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets.h"

#define V_PADDING_DEFAULT 8
#define V_PADDING_COMPACT 4

#define H_PADDING_DEFAULT 4

#define SIDEBAR_WIDTH 30
#define HORIZONTAL_BAR_HEIGHT FIXED_WIDGET_HEIGHT

GRect screen_rect;

// "private" functions
// layer update callbacks
#ifndef PBL_ROUND
  void updateRectSidebar(Layer *l, GContext* ctx);
#else

  void updateRoundSidebarLeft(Layer *l, GContext* ctx);
  void updateRoundSidebarRight(Layer *l, GContext* ctx);

  // shared drawing stuff between all layers
  void drawRoundSidebar(GContext* ctx, GRect bgBounds, SidebarWidgetType widgetType, int widgetXOffset);
#endif

Layer* sidebarLayer;

#ifdef PBL_ROUND
  Layer* sidebarLayer2;
#endif

void Sidebar_init(Window* window) {
  // init the sidebar layer
  screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds;

  #ifdef PBL_ROUND
    GRect bounds2;
    bounds = GRect(0, 0, 40, screen_rect.size.h);
    bounds2 = GRect(screen_rect.size.w - 40, 0, 40, screen_rect.size.h);
  #else
    if(globalSettings.sidebarLocation == RIGHT) {
      bounds = GRect(screen_rect.size.w - SIDEBAR_WIDTH, 0, SIDEBAR_WIDTH, screen_rect.size.h);
    } else if(globalSettings.sidebarLocation == LEFT) {
      bounds = GRect(0, 0, SIDEBAR_WIDTH, screen_rect.size.h);
    } else if(globalSettings.sidebarLocation == BOTTOM) {
      bounds = GRect(0, screen_rect.size.h - HORIZONTAL_BAR_HEIGHT, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT);
    } else if(globalSettings.sidebarLocation == TOP) {
      bounds = GRect(0, 0, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT);
    }else {
      bounds = GRect(0, 0, 0, 0);
    }
  #endif

  // init the widgets
  SidebarWidgets_init();

  sidebarLayer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), sidebarLayer);

  #ifdef PBL_ROUND
    layer_set_update_proc(sidebarLayer, updateRoundSidebarLeft);
  #else
    layer_set_update_proc(sidebarLayer, updateRectSidebar);
  #endif

  #ifdef PBL_ROUND
    sidebarLayer2 = layer_create(bounds2);
    layer_add_child(window_get_root_layer(window), sidebarLayer2);
    layer_set_update_proc(sidebarLayer2, updateRoundSidebarRight);
  #endif
}

void Sidebar_deinit() {
  layer_destroy(sidebarLayer);

  #ifdef PBL_ROUND
    layer_destroy(sidebarLayer2);
  #endif

  SidebarWidgets_deinit();
}

void Sidebar_redraw() {
  #ifndef PBL_ROUND
    // reposition the sidebar if needed
    if(globalSettings.sidebarLocation == RIGHT) {
      layer_set_frame(sidebarLayer, GRect(screen_rect.size.w - SIDEBAR_WIDTH, 0, SIDEBAR_WIDTH, screen_rect.size.h));
    } else if(globalSettings.sidebarLocation == LEFT) {
      layer_set_frame(sidebarLayer, GRect(0, 0, SIDEBAR_WIDTH, screen_rect.size.h));
    } else if(globalSettings.sidebarLocation == BOTTOM) {
      layer_set_frame(sidebarLayer, GRect(0, screen_rect.size.h - HORIZONTAL_BAR_HEIGHT, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT));
    } else if(globalSettings.sidebarLocation == TOP) {
      layer_set_frame(sidebarLayer, GRect(0, 0, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT));
    } else {
      layer_set_frame(sidebarLayer, GRect(0, 0, 0, 0));
    }
  #endif

  // redraw the layer
  layer_mark_dirty(sidebarLayer);

  #ifdef PBL_ROUND
    layer_mark_dirty(sidebarLayer2);
  #endif
}

void Sidebar_updateTime(struct tm* timeInfo) {
  SidebarWidgets_updateTime(timeInfo);

  // redraw the sidebar in case it changed in any way
  Sidebar_redraw();
}

bool isAutoBatteryShown() {
  if(!globalSettings.disableAutobattery) {
    BatteryChargeState chargeState = battery_state_service_peek();

    if(globalSettings.enableAutoBatteryWidget) {
      if(chargeState.charge_percent <= 10 || chargeState.is_charging) {
        return true;
      }
    }
  }

  return false;
}


#ifdef PBL_ROUND

// returns the best candidate widget for replacement by the auto battery
// or the disconnection icon
int getReplacableWidget() {
  if(globalSettings.widgets[0] == EMPTY) {
    return 0;
  } else if(globalSettings.widgets[2] == EMPTY) {
    return 2;
  }

  if(globalSettings.widgets[0] == WEATHER_CURRENT || globalSettings.widgets[0] == WEATHER_FORECAST_TODAY) {
    return 0;
  } else if(globalSettings.widgets[2] == WEATHER_CURRENT || globalSettings.widgets[2] == WEATHER_FORECAST_TODAY) {
    return 2;
  }

  // if we don't have any of those things, just replace the left widget
  return 0;
}

#else

// returns the best candidate widget for replacement by the auto battery
// or the disconnection icon
int getReplacableWidget() {
  // if any widgets are empty, it's an obvious choice
  for(int i = 0; i < 3; i++) {
    if(globalSettings.widgets[i] == EMPTY) {
      return i;
    }
  }

  // use widget 4 only if bottom or top widget is used
  if(globalSettings.widgets[3] == EMPTY &&
     (globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP)) {
    return 3;
  }

  // are there any bluetooth-enabled widgets? if so, they're the second-best
  // candidates
  for(int i = 0; i < 4; i++) {
    if(globalSettings.widgets[i] == WEATHER_CURRENT || globalSettings.widgets[i] == WEATHER_FORECAST_TODAY) {
      return i;
    }
  }

  // if we don't have any of those things, just replace the middle widget
  return 1;
}

#endif

#ifdef PBL_ROUND

void updateRoundSidebarRight(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  bool showDisconnectIcon = !bluetooth_connection_service_peek();
  bool showAutoBattery = isAutoBatteryShown();

  SidebarWidgetType displayWidget = globalSettings.widgets[2];

  if((showAutoBattery || showDisconnectIcon) && getReplacableWidget() == 2) {
    if(showAutoBattery) {
      displayWidget = BATTERY_METER;
    } else if(showDisconnectIcon) {
      displayWidget = BLUETOOTH_DISCONNECT;
    }
  }

  drawRoundSidebar(ctx, bgBounds, displayWidget, 3);
}

void updateRoundSidebarLeft(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x - bounds.size.h * 2 + bounds.size.w, bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  bool showDisconnectIcon = !bluetooth_connection_service_peek();
  bool showAutoBattery = isAutoBatteryShown();
  SidebarWidgetType displayWidget = globalSettings.widgets[0];

  if((showAutoBattery || showDisconnectIcon) && getReplacableWidget() == 0) {
    if(showAutoBattery) {
      displayWidget = BATTERY_METER;
    } else if(showDisconnectIcon) {
      displayWidget = BLUETOOTH_DISCONNECT;
    }
  }

  drawRoundSidebar(ctx, bgBounds, displayWidget, 7);
}

void drawRoundSidebar(GContext* ctx, GRect bgBounds, SidebarWidgetType widgetType, int widgetXOffset) {
  SidebarWidgets_updateFonts();

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  graphics_fill_radial(ctx,
                       bgBounds,
                       GOvalScaleModeFillCircle,
                       100,
                       DEG_TO_TRIGANGLE(0),
                       TRIG_MAX_ANGLE);

  SidebarWidgets_xOffset = widgetXOffset;
  SidebarWidget widget = getSidebarWidgetByType(widgetType);

  // calculate center position of the widget
  int widgetPosition = bgBounds.size.h / 4 - widget.getHeight() / 2;

  widget.draw(ctx, 0, widgetPosition);
}

#else

void updateRectSidebar(Layer *l, GContext* ctx) {
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(l);
  GRect bounds = layer_get_bounds(l);

  SidebarWidgets_updateFonts();

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  bool showDisconnectIcon = false;
  bool showAutoBattery = isAutoBatteryShown();

  // if the pebble is disconnected and activated, show the disconnect icon
  if(globalSettings.activateDisconnectIcon) {
    showDisconnectIcon = !bluetooth_connection_service_peek();
  }

  SidebarWidget displayWidgets[4];

  displayWidgets[0] = getSidebarWidgetByType(globalSettings.widgets[0]);
  displayWidgets[1] = getSidebarWidgetByType(globalSettings.widgets[1]);
  displayWidgets[2] = getSidebarWidgetByType(globalSettings.widgets[2]);
  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
    displayWidgets[3] = getSidebarWidgetByType(globalSettings.widgets[3]);
  }

  // do we need to replace a widget?
  // if so, determine which widget should be replaced
  if(showAutoBattery || showDisconnectIcon) {
    int widget_to_replace = getReplacableWidget();

    if(showAutoBattery) {
      displayWidgets[widget_to_replace] = getSidebarWidgetByType(BATTERY_METER);
    } else if(showDisconnectIcon) {
      displayWidgets[widget_to_replace] = getSidebarWidgetByType(BLUETOOTH_DISCONNECT);
    }
  }

  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
    // calculate the three horizontal widget positions
    int leftWidgetPos = H_PADDING_DEFAULT;
    int middleWidgetPos = (bounds.size.w - SIDEBAR_WIDTH) / 2;
    int rightWidgetPos = bounds.size.w - H_PADDING_DEFAULT - SIDEBAR_WIDTH;
    int v_padding;

    // use compact mode and fixed height for bottom and top widget
    SidebarWidgets_useCompactMode = true;
    SidebarWidgets_fixedHeight = true;

    // draw the widgets
    v_padding= (HORIZONTAL_BAR_HEIGHT - displayWidgets[0].getHeight()) / 2;
    displayWidgets[0].draw(ctx, leftWidgetPos, v_padding);

    if(globalSettings.widgets[3] == EMPTY) {
      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[1].getHeight()) / 2;
      displayWidgets[1].draw(ctx, middleWidgetPos, v_padding);

      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[2].getHeight()) / 2;
      displayWidgets[2].draw(ctx, rightWidgetPos, v_padding);
    }else if(globalSettings.widgets[2] == EMPTY) {
      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[1].getHeight()) / 2;
      displayWidgets[1].draw(ctx, middleWidgetPos, v_padding);

      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[3].getHeight()) / 2;
      displayWidgets[3].draw(ctx, rightWidgetPos, v_padding);
    }else if(globalSettings.widgets[1] == EMPTY) {
      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[2].getHeight()) / 2;
      displayWidgets[2].draw(ctx, middleWidgetPos, v_padding);

      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[3].getHeight()) / 2;
      displayWidgets[3].draw(ctx, rightWidgetPos, v_padding);
    } else { // we have 4 widgets

      // middle position 1
      middleWidgetPos = (bounds.size.w - 5 * H_PADDING_DEFAULT) / 4 + 2 * H_PADDING_DEFAULT;
      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[1].getHeight()) / 2;
      displayWidgets[1].draw(ctx, middleWidgetPos, v_padding);

      // middle position 2
      middleWidgetPos = (bounds.size.w - 5 * H_PADDING_DEFAULT) / 2 + 3 * H_PADDING_DEFAULT;
      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[2].getHeight()) / 2;
      displayWidgets[2].draw(ctx, middleWidgetPos, v_padding);

      v_padding = (HORIZONTAL_BAR_HEIGHT - displayWidgets[3].getHeight()) / 2;
      displayWidgets[3].draw(ctx, rightWidgetPos, v_padding);
    }
  } else if(globalSettings.sidebarLocation != NONE) {

    // if the widgets are too tall, enable "compact mode"
    int compact_mode_threshold = unobstructed_bounds.size.h - V_PADDING_DEFAULT * 2 - 3;
    int v_padding = V_PADDING_DEFAULT;

    SidebarWidgets_useCompactMode = false; // ensure that we compare the non-compacted heights
    SidebarWidgets_fixedHeight = false;
    int totalHeight = displayWidgets[0].getHeight() + displayWidgets[1].getHeight() + displayWidgets[2].getHeight();
    SidebarWidgets_useCompactMode = (totalHeight > compact_mode_threshold);
    // printf("Total Height: %i, Threshold: %i", totalHeight, compact_mode_threshold);

    // now that they have been compacted, check if they fit a second time,
    // if they still don't fit, our only choice is MURDER (of the middle widget)
    totalHeight = displayWidgets[0].getHeight() + displayWidgets[1].getHeight() + displayWidgets[2].getHeight();
    bool hide_middle_widget = (totalHeight > compact_mode_threshold);
    // printf("Compact Mode Enabled. Total Height: %i, Threshold: %i", totalHeight, compact_mode_threshold);

    // still doesn't fit? try compacting the vertical padding
    totalHeight = displayWidgets[0].getHeight() + displayWidgets[2].getHeight();
    if(totalHeight > compact_mode_threshold) {
      v_padding = V_PADDING_COMPACT;
    }

    // calculate the three widget positions
    int topWidgetPos = v_padding;
    int lowerWidgetPos = unobstructed_bounds.size.h - v_padding - displayWidgets[2].getHeight();

    // vertically center the middle widget using MATH
    int middleWidgetPos = ((lowerWidgetPos - displayWidgets[1].getHeight()) + (topWidgetPos + displayWidgets[0].getHeight())) / 2;

    // draw the widgets
    displayWidgets[0].draw(ctx, 0, topWidgetPos);
    if(!hide_middle_widget) {
      displayWidgets[1].draw(ctx, 0, middleWidgetPos);
    }
    displayWidgets[2].draw(ctx, 0, lowerWidgetPos);
  }
}

#endif
