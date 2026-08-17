#ifndef Smoothing_h
#define Smoothing_h

// Paired with SENSOR_TIMING_BUDGET in ESPRoto.cpp -- raise that before this,
// integration is quieter per ms of latency than averaging after the fact.
#define NUM_READINGS 4

class Smoothing {
  public:
    // Parameters
    int readings[NUM_READINGS] = {0};

    // Methods
    Smoothing();
    int filter(int input);
  
  private:
    long total;
    int readIndex;
};

#endif