#include <string.h>
#include "Smoothing.h"

Smoothing::Smoothing() {
  readIndex = 0;
#ifndef FILTER_MEDIAN
  total = 0;
#endif
}

int Smoothing::filter(int input) {
#ifdef FILTER_MEDIAN
  readings[readIndex] = input;
  if (++readIndex >= NUM_READINGS) readIndex = 0;

  // Insertion sort of a copy: NUM_READINGS is single digits, so this beats
  // anything cleverer and leaves the ring buffer in arrival order.
  int sorted[NUM_READINGS];
  memcpy(sorted, readings, sizeof(sorted));
  for (int i = 1; i < NUM_READINGS; i++) {
    int v = sorted[i];
    int j = i - 1;
    while (j >= 0 && sorted[j] > v) {
      sorted[j + 1] = sorted[j];
      j--;
    }
    sorted[j + 1] = v;
  }
  return sorted[NUM_READINGS / 2];
#else
  // Moving average
  total += input - readings[readIndex];
  readings[readIndex] = input;
  if (++readIndex >= NUM_READINGS) readIndex = 0;
  return (int)(total / NUM_READINGS);
#endif
}
