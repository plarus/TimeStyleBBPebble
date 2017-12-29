#include <pebble.h>
#include <ctype.h>
#include "settings.h"
#include "weather.h"
#include "languages.h"
#include "sidebar.h"
#include "sidebar_widgets.h"
#include "util.h"

#define V_PADDING_DEFAULT 8
#define V_PADDING_COMPACT 4
#define COMPACT_MODE_THRESHOLD 142
#define SCREEN_HEIGHT 168

#define H_PADDING_DEFAULT 4
#define HORIZONTAL_BAR_HEIGHT FIXED_WIDGET_HEIGHT

static GRect screen_rect;
static Layer* sidebarLayer;

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

  int v_padding;
  int middleWidgetPos;

  if(globalSettings.sidebarLocation == BOTTOM || globalSettings.sidebarLocation == TOP) {
    // calculate the three horizontal widget positions
    middleWidgetPos = (bounds.size.w - ACTION_BAR_WIDTH) / 2;
    int rightWidgetPos = bounds.size.w - H_PADDING_DEFAULT - ACTION_BAR_WIDTH;

    // use compact mode and fixed height for bottom and top widget
    SidebarWidgets_useCompactMode = true;
    SidebarWidgets_fixedHeight = true;

    // draw the widgets
    v_padding= (HORIZONTAL_BAR_HEIGHT - displayWidgets[0].getHeight()) / 2;
    displayWidgets[0].draw(ctx, H_PADDING_DEFAULT, v_padding);

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
  } else if(globalSettings.sidebarLocation == LEFT || globalSettings.sidebarLocation == RIGHT) {
    v_padding = V_PADDING_DEFAULT;
    // if the widgets are too tall, enable "compact mode"
    SidebarWidgets_useCompactMode = false; // ensure that we compare the non-compacted heights
    SidebarWidgets_fixedHeight = false;
    int totalHeight = displayWidgets[0].getHeight() + displayWidgets[1].getHeight() + displayWidgets[2].getHeight();
    SidebarWidgets_useCompactMode = (totalHeight > COMPACT_MODE_THRESHOLD);

    // now that they have been compacted, check if they fit a second time,
    // if they still don't fit, our only choice is MURDER (of the middle widget)
    totalHeight = displayWidgets[0].getHeight() + displayWidgets[1].getHeight() + displayWidgets[2].getHeight();
    bool hide_middle_widget = (totalHeight > COMPACT_MODE_THRESHOLD);

    // do not use compact mode if middle widget is hidden (biggest widget height is smaller than 168/2)
    if(hide_middle_widget) {
      SidebarWidgets_useCompactMode = false;
    }

    // still doesn't fit? try compacting the vertical padding
    totalHeight = displayWidgets[0].getHeight() + displayWidgets[2].getHeight();
    if(totalHeight > COMPACT_MODE_THRESHOLD) {
      v_padding = V_PADDING_COMPACT;
    }

    // draw the widgets
    int lowerWidgetPos = SCREEN_HEIGHT - v_padding - displayWidgets[2].getHeight();
    displayWidgets[0].draw(ctx, 0, v_padding);
    if(!hide_middle_widget) {
      // vertically center the middle widget using MATH
      middleWidgetPos = ((lowerWidgetPos - displayWidgets[1].getHeight()) + (v_padding + displayWidgets[0].getHeight())) / 2;
      displayWidgets[1].draw(ctx, 0, middleWidgetPos);
    }
    displayWidgets[2].draw(ctx, 0, lowerWidgetPos);
  }
}

void Sidebar_init(Window* window) {
  // init the sidebar layer
  screen_rect = layer_get_bounds(window_get_root_layer(window));
  GRect bounds;

  bounds = getRectSidebarBounds();

  // init the widgets
  SidebarWidgets_init();

  sidebarLayer = layer_create(bounds);
  layer_add_child(window_get_root_layer(window), sidebarLayer);

  layer_set_update_proc(sidebarLayer, updateRectSidebar);
}

void Sidebar_deinit(void) {
  layer_destroy(sidebarLayer);

  SidebarWidgets_deinit();
}

void Sidebar_set_layer(void) {
  // reposition the sidebar if needed
  layer_set_frame(sidebarLayer, getRectSidebarBounds());

  SidebarWidgets_updateFonts();
}

void Sidebar_redraw() {
  // redraw the layer
  layer_mark_dirty(sidebarLayer);
}
