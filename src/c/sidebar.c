#include <pebble.h>
#include <ctype.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets.h"
#include "util.h"

#define V_PADDING 8
#define SCREEN_HEIGHT 168

#define H_PADDING_DEFAULT 4
#define HORIZONTAL_BAR_HEIGHT FIXED_WIDGET_HEIGHT

static GRect screen_rect;
static Layer* sidebarLayer;

#ifdef PBL_ROUND
  static Layer* sidebarLayer2;
#endif

static bool isAutoBatteryShown(void) {
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
static int getReplacableWidget(void) {
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
static int getReplacableWidget(void) {
  // if any widgets are empty, it's an obvious choice
  for(int i = 0; i < 3; i++) {
    if(globalSettings.widgets[i] == EMPTY) {
      return i;
    }
  }

  // are there any bluetooth-enabled widgets? if so, they're the second-best
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
static SidebarWidget getRoundSidebarWidget(int widgetNumber) {
  bool showDisconnectIcon = !bluetooth_connection_service_peek();
  bool showAutoBattery = isAutoBatteryShown();

  SidebarWidgetType displayWidget = globalSettings.widgets[widgetNumber];

  if((showAutoBattery || showDisconnectIcon) && getReplacableWidget() == widgetNumber) {
    if(showAutoBattery) {
      displayWidget = BATTERY_METER;
    } else if(showDisconnectIcon) {
      displayWidget = BLUETOOTH_DISCONNECT;
    }
  }

  return getSidebarWidgetByType(displayWidget);
}

static void drawRoundSidebar(GContext* ctx, GRect bgBounds, SidebarWidget widget, int widgetXPosition, int widgetYPosition, int widgetXOffset) {
  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);

  graphics_fill_radial(ctx,
                       bgBounds,
                       GOvalScaleModeFillCircle,
                       100,
                       DEG_TO_TRIGANGLE(0),
                       TRIG_MAX_ANGLE);

  SidebarWidgets_xOffset = widgetXOffset;

  widget.draw(ctx, widgetXPosition, widgetYPosition);
}

static void updateRoundSidebarRight(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x, bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  SidebarWidget widget = getRoundSidebarWidget(2);

  // calculate center position of the widget
  int widgetYPosition = bgBounds.size.h / 4 - widget.getHeight() / 2;

  drawRoundSidebar(ctx, bgBounds, widget, 0, widgetYPosition, 3);
  }

static void updateRoundSidebarLeft(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.origin.x - bounds.size.h * 2 + bounds.size.w, bounds.size.h / -2, bounds.size.h * 2, bounds.size.h * 2);

  SidebarWidget widget = getRoundSidebarWidget(0);

  // calculate center position of the widget
  int widgetYPosition = bgBounds.size.h / 4 - widget.getHeight() / 2;

  drawRoundSidebar(ctx, bgBounds, widget, 0, widgetYPosition, 7);
  }

static void updateRoundSidebarBottom(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.size.w / -2, bounds.origin.y, bounds.size.w * 2, bounds.size.w * 2);

  SidebarWidget widget = getRoundSidebarWidget(2);

  // use compact mode and fixed height for bottom and top widget
  SidebarWidgets_useCompactMode = true;
  SidebarWidgets_fixedHeight = true;

  // calculate center position of the widget
  int widgetXPosition = bgBounds.size.w / 4 - ACTION_BAR_WIDTH / 2;
  int widgetYPosition = (HORIZONTAL_BAR_HEIGHT - widget.getHeight()) / 2;

  drawRoundSidebar(ctx, bgBounds, widget, widgetXPosition, widgetYPosition, 5);
}

static void updateRoundSidebarTop(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);
  GRect bgBounds = GRect(bounds.size.w / -2, bounds.origin.y - bounds.size.w * 2 + bounds.size.h, bounds.size.w * 2, bounds.size.w * 2);

  SidebarWidget widget = getRoundSidebarWidget(0);

  // use compact mode and fixed height for bottom and top widget
  SidebarWidgets_useCompactMode = true;
  SidebarWidgets_fixedHeight = true;

  // calculate center position of the widget
  int widgetXPosition = bgBounds.size.w / 4 - ACTION_BAR_WIDTH / 2;
  int widgetYPosition = (HORIZONTAL_BAR_HEIGHT - widget.getHeight()) / 2;

  drawRoundSidebar(ctx, bgBounds, widget, widgetXPosition, widgetYPosition, 5);
}

#else

static GRect getRectSidebarBounds(void) {
  if(globalSettings.sidebarLocation == RIGHT) {
    return GRect(114, 0, 30, screen_rect.size.h);
  } else if(globalSettings.sidebarLocation == LEFT) {
    return GRect(0, 0, 30, screen_rect.size.h);
  } else if(globalSettings.sidebarLocation == BOTTOM) {
    return GRect(0, screen_rect.size.h - HORIZONTAL_BAR_HEIGHT, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT);
  } else if(globalSettings.sidebarLocation == TOP) {
    return GRect(0, 0, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT);
  }else {
    return GRect(0, 0, 0, 0);
  }
}

static void updateRectSidebar(Layer *l, GContext* ctx) {
  GRect bounds = layer_get_bounds(l);

  graphics_context_set_fill_color(ctx, globalSettings.sidebarColor);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_text_color(ctx, globalSettings.sidebarTextColor);

  // if the pebble is disconnected, show the disconnect icon
  //bool showDisconnectIcon = !bluetooth_connection_service_peek();
  bool showDisconnectIcon = false; // TODO : Add configurable parameter
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
    int middleWidgetPos = (bounds.size.w - ACTION_BAR_WIDTH) / 2;
    int rightWidgetPos = bounds.size.w - H_PADDING_DEFAULT - ACTION_BAR_WIDTH;
    int v_padding;

    // use compact mode and fixed height for bottom and top widget
    SidebarWidgets_useCompactMode = true;
    SidebarWidgets_fixedHeight = true;

    // draw the widgets
    v_padding= (HORIZONTAL_BAR_HEIGHT - displayWidgets[0].getHeight()) / 2;
    displayWidgets[0].draw(ctx, leftWidgetPos, v_padding);

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
  } else if(globalSettings.sidebarLocation == LEFT || globalSettings.sidebarLocation == RIGHT) {
    // if the widgets are too tall, enable "compact mode"
    SidebarWidgets_useCompactMode = false; // ensure that we compare the non-compacted heights
    SidebarWidgets_fixedHeight = false;
    int totalHeight = displayWidgets[0].getHeight() + displayWidgets[1].getHeight() + displayWidgets[2].getHeight();
    SidebarWidgets_useCompactMode = (totalHeight > 142) ? true : false;

    // calculate the three widget positions
    int topWidgetPos = V_PADDING;
    int lowerWidgetPos = SCREEN_HEIGHT - V_PADDING - displayWidgets[2].getHeight();

    // vertically center the middle widget using MATH
    int middleWidgetPos = ((lowerWidgetPos - displayWidgets[1].getHeight()) + (topWidgetPos + displayWidgets[0].getHeight())) / 2;

    // draw the widgets
    displayWidgets[0].draw(ctx, 0, topWidgetPos);
    displayWidgets[1].draw(ctx, 0, middleWidgetPos);
    displayWidgets[2].draw(ctx, 0, lowerWidgetPos);
  }
}
#endif

void Sidebar_init(Window* window) {
  // init the sidebar layer
  screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds;

  #ifdef PBL_ROUND
    GRect bounds2;
    //bounds = GRect(0, 0, 40, screen_rect.size.h);
    //bounds2 = GRect(screen_rect.size.w - 40, 0, 40, screen_rect.size.h);
    bounds = GRect(0, 0, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT );
    bounds2 = GRect(0, screen_rect.size.h - HORIZONTAL_BAR_HEIGHT, screen_rect.size.w, HORIZONTAL_BAR_HEIGHT);
  #else
    bounds = getRectSidebarBounds();
  #endif

  // init the widgets
  SidebarWidgets_init();

  sidebarLayer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), sidebarLayer);

  #ifdef PBL_ROUND
    //layer_set_update_proc(sidebarLayer, updateRoundSidebarLeft);
    layer_set_update_proc(sidebarLayer, updateRoundSidebarTop);
  #else
    layer_set_update_proc(sidebarLayer, updateRectSidebar);
  #endif

  #ifdef PBL_ROUND
    sidebarLayer2 = layer_create(bounds2);
    layer_add_child(window_get_root_layer(window), sidebarLayer2);
    //layer_set_update_proc(sidebarLayer2, updateRoundSidebarRight);
    layer_set_update_proc(sidebarLayer2, updateRoundSidebarBottom);
  #endif
}

void Sidebar_deinit(void) {
  layer_destroy(sidebarLayer);

  #ifdef PBL_ROUND
    layer_destroy(sidebarLayer2);
  #endif

  SidebarWidgets_deinit();
}

void Sidebar_set_layer(void) {
  #ifndef PBL_ROUND
    // reposition the sidebar if needed
    layer_set_frame(sidebarLayer, getRectSidebarBounds());
  #endif

  SidebarWidgets_updateFonts();
}

void Sidebar_redraw() {
  // redraw the layer
  layer_mark_dirty(sidebarLayer);

  #ifdef PBL_ROUND
    layer_mark_dirty(sidebarLayer2);
  #endif
}

