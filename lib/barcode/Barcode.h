// a C++ class that holds softcut voices

#ifndef BARCODE_H
#define BARCODE_H

#include "../softcut/Voices.h"
using namespace softcut;

class Barcode {
 public:
  // make constructor with pointer to voices
  Barcode(){};
  void init(float *tape, unsigned int numFrames, float sr) {
    voices.init(tape, numFrames, sr);
    // make all voices uses the same tape
    for (size_t i = 0; i < NUM_VOICES; i++) {
      voices.setTape(i, 0);
      voices.setPlayFlag(i, true);
    }
    // set first voice to play and record
    voices.setRecFlag(0, true);
    voices.setPlayFlag(0, true);
    // set all the
  };

 private:
  Voices voices;
};

#endif