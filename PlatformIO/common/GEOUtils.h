#pragma once

#include <Arduino.h>
#include <math.h>

// ---- Macros ----

#define SCALE_TO_MIDI(val, min, max) map(constrain((val), (min), (max)), (min), (max), 0, 127)
#define FLOAT_TO_MIDI(val) (int)constrain(floor((val) * 127.0), 0, 127)

// ---- Functions ----
int normalize_distance(int distance, int radius, int angle) {
  int difference = radius - radius / cos(DEG_TO_RAD * angle);
  return distance - difference;
}