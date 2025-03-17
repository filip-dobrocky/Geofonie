#include "Smoothing.h"

Smoothing::Smoothing() {
  readIndex = 0;
  total = 0;
}

int Smoothing::filter(int input) {
 	////Perform average on sensor readings
 	long average;
 	// subtract the last reading:
 	total = total - readings[readIndex];
 	
	readings[readIndex] = input;
 	// add value to total:
 	total = total + readings[readIndex];
 	// handle index
 	readIndex++;
 	if (readIndex >= NUM_READINGS) {
 			readIndex = 0;
 	}
 	// calculate the average:
 	average = total / NUM_READINGS;

 	return average;
}