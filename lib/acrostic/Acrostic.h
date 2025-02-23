#ifndef ACROSTIC_H
#define ACROSTIC_H

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "../App.h"
#include "../Config.h"
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

class Acrostic : public App {
 public:
  enum class CallbackType {
    ON_PLAY_START,
    ON_PLAY_STOP,
    ON_RECORD_START,
    ON_RECORD_STOP,
    ON_PLAY,
    NUM_CALLBACKS,
  };
  // make constructor with pointer to voices
  Acrostic() = default;
  void Init(float *tape, unsigned int numFrames, float sr,
            float audioblockSize) override;
  void Process(const float *const *in, float **out,
               unsigned int numFrames) override;
  Voices &getVoices() override { return voices; }
  void setMainWet(float val) { voices.setMainWet(val); }
  void RecordLoop(uint8_t voice);
  void SetBPM(float bpm) { bpm_ = bpm; }

 private:
  Voices voices;
  Oscillator2 osc[CONFIG_VOICE_NUM][NUM_OSCILLATORS];
  float bpm_;
};

#endif