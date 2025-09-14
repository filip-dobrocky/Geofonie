#ifndef Smoothing_h
#define Smoothing_h

#define NUM_READINGS 10

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