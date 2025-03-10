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
  enum class CallbackType {
    ON_PLAY_START,
    ON_PLAY_STOP,
    ON_RECORD_START,
    ON_RECORD_STOP,
    ON_PLAY,
    ON_RECORD
  };
  // make constructor with pointer to voices
  Barcode() = default;
  void Init(float *tape, unsigned int numFrames, float sr,
            float audioblockSize) override;
  void Process(const float *const *in, float **out,
               unsigned int numFrames) override;
  Voices &getVoices() override { return voices; }
  void setMainWet(float val) { voices.setMainWet(val); }

  void ToggleRecording(bool on);
  void ToggleRecording();
  void TogglePlaying(bool on);

 private:
  void setPlaying(bool on) {
    playing = on;
    if (on) {
      triggerCallback(static_cast<int>(CallbackType::ON_PLAY_START));
    } else {
      triggerCallback(static_cast<int>(CallbackType::ON_PLAY_STOP));
    }
  }
  void setRecording(bool on) {
    recording = on;
    if (on) {
      triggerCallback(static_cast<int>(CallbackType::ON_RECORD_START));
    } else {
      triggerCallback(static_cast<int>(CallbackType::ON_RECORD_STOP));
    }
  }
  Voices voices;
  float recordingStop = 0;
  bool recording;
  bool playing;
  int xfadeSamplesWait = 0;
  int xfadeSamples = 128 * 10;
  float xfadeSeconds = 0.1;
  Oscillator2 osc[CONFIG_VOICE_NUM][NUM_OSCILLATORS];
  // {0.125,0.25,0.5,1,2,4}
  // 0.6    0.6  1   1 0.1 0.05
  float rates[7] = {1, 0.125, 0.25, 0.5, 2, 1.5, 4};
  float dbs[7] = {0, -5, -5, 0, -12, -16, -24};
};

#endif