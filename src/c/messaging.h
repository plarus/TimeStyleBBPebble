#pragma once
#include <pebble.h>

typedef void (*MessageProcessedCallback)(void);

void messaging_requestNewWeatherData(void);
void messaging_init(MessageProcessedCallback callback);
