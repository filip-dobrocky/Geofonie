#pragma once

// ---- Macros ----

#define SCALE_TO_MIDI(val, min, max) map(constrain((val), (min), (max)), (min), (max), 0, 127)
#define FLOAT_TO_MIDI(val) (int)constrain(floor((val) * 127.0), 0, 127)