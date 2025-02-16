#ifndef BARCODE_H
#define BARCODE_H

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "../dsp/Oscillator.h"
#include "../softcut/Utilities.h"
#include "../softcut/Voices.h"
using namespace softcut;

enum OscillatorIndex {
  LFO_AMP,
  LFO_PAN,
  LFO_DIRECTION,
  LFO_START,
  LFO_LENGTH,
  NUM_OSCILLATORS
};

class Barcode {
 public:
  // make constructor with pointer to voices
  Barcode(){};
  void init(float *tape, unsigned int numFrames, float sr,
            float audioblockSize);
  void ToggleRecording(bool on);
  void process(const float *inl, const float *inr, float *outl, float *outr,
               unsigned int numFrames);

 private:
  Voices voices;
  float recordingStop = 0;
  bool recording;
  bool barcoding;
  const float xfadeSeconds = 0.1;
  int xfadeSamplesWait = 0;
  int xfadeSamples = 1000;
  Oscillator osc[NUM_VOICES][NUM_OSCILLATORS];
  float rates[6] = {1.0, 0.5, 1.5, 0.25, 4.0, 8.0};
  float dbs[6] = {0, 6, -6, 12, 0, 0};
};

#endif