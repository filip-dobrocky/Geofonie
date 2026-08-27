#ifndef Smoothing_h
#define Smoothing_h

// Sample window, shared by both modes. Paired with SENSOR_TIMING_BUDGET in
// ESPRoto.cpp -- raise that before this, integration is quieter per ms of
// latency than filtering after the fact. Override with -DNUM_READINGS=n.
#ifndef NUM_READINGS
#define NUM_READINGS 4
#endif

// -DFILTER_MEDIAN swaps the moving average for a median: rejects VL53L4CD
// outliers outright instead of smearing them, and keeps target edges sharp.
// Use an odd NUM_READINGS with it, an even window has no true middle sample.
#if defined(FILTER_MEDIAN) && (NUM_READINGS % 2 == 0)
#warning "FILTER_MEDIAN with an even NUM_READINGS is biased high; use an odd window"
#endif

class Smoothing {
  public:
    // Parameters
    int readings[NUM_READINGS] = {0};

    // Methods
    Smoothing();
    int filter(int input);

  private:
    int readIndex;
#ifndef FILTER_MEDIAN
    long total;
#endif
};

#endif
