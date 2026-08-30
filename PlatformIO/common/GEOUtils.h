#pragma once

#include <Arduino.h>
#include <math.h>

// ---- Macros ----

#define SCALE_TO_MIDI(val, min, max) scale_to_midi((val), (min), (max))
#define FLOAT_TO_MIDI(val) (int)constrain(floor((val) * 127.0), 0, 127)

// ---- Functions ----

// map() divides by (max - min); a collapsed range is an integer divide by zero,
// which on Xtensa is a panic, not a NaN.
inline int scale_to_midi(long val, long min, long max) {
  if (max <= min) return 0;
  return map(constrain(val, min, max), min, max, 0, 127);
}
int normalize_distance(int distance, int radius, int angle) {
  int difference = radius - radius / cos(DEG_TO_RAD * angle);
  return distance - difference;
}