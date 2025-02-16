#ifndef BARCODE_H
#define BARCODE_H

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "../App.h"
#include "../dsp/Oscillator2.h"
#include "../softcut/Utilities.h"
#include "../softcut/Voices.h"

using namespace softcut;

enum Oscillator2Index {
  LFO_AMP,
  LFO_PAN,
  LFO_DIRECTION,
  LFO_START,
  LFO_LENGTH,
  NUM_OSCILLATORS
};

class Barcode : public App {
 public:
  // make constructor with pointer to voices
  Barcode() = default;
  void Init(float *tape, unsigned int numFrames, float sr,
            float audioblockSize) override;
  void Process(const float *inl, const float *inr, float *outl, float *outr,
               unsigned int numFrames) override;
  Voices &getVoices() override { return voices; }
  void ToggleRecording(bool on);

  bool Barcoding() { return barcoding; }
  bool Recording() { return recording; }
  float VoicePosition(size_t voice) { return voices.getSavedPosition(voice); }
  void setMainWet(float val) { voices.setMainWet(val); }

 private:
  Voices voices;
  float recordingStop = 0;
  bool recording;
  bool barcoding;
  int xfadeSamplesWait = 0;
  int xfadeSamples = 128 * 10;
  float xfadeSeconds = 0.1;
  Oscillator2 osc[NUM_VOICES][NUM_OSCILLATORS];
  float rates[6] = {1.0, 0.5, 1.5, 0.25, 1.0, 2.0};
  float dbs[6] = {6, 12, 0, 12, 21, -3};
};

#endif