#ifndef SOFTCUT_VOICES_H
#define SOFTCUT_VOICES_H

#include "Utilities.h"
#include "Voice.h"

#define NUM_VOICES 5

namespace softcut {

class Voices {
 public:
  Voices() : wetDrySlew(48000) {
    for (size_t i = 0; i < NUM_VOICES; i++) {
      levelSlew[i] = LinearRamp(48000);
    }
  }

  void init(float *buf, unsigned int numFrames, float sampleRate,
            float audioblockSize);
  void setTape(size_t voice, size_t index);
  void setRate(size_t voice, float rate);
  void setDB(size_t voice, float db);
  void setLevel(size_t voice, float level);
  void setPan(size_t voice, float pan);
  void setLoopStart(size_t voice, float sec);

  void setLoopStart(float sec) {
    for (size_t i = 0; i < NUM_VOICES; i++) {
      setLoopStart(i, sec);
    }
  }

  void setLoopEnd(size_t voice, float sec);
  void setLoopEnd(float sec);
  void setLoopFlag(size_t voice, bool val);
  void setRecFlag(size_t voice, bool val);
  void setRecOnceFlag(size_t voice, bool val);
  void setPlayFlag(size_t voice, bool val);
  void setPreLevel(size_t voice, float val);
  void setRecLevel(size_t voice, float val);
  void cutToPos(size_t voice, float sec);
  void cutToPos(float sec);

  void setFadeTime(size_t voice, float sec) { voices[voice].setFadeTime(sec); }

  void setFadeTime(float sec) {
    for (size_t i = 0; i < NUM_VOICES; i++) {
      voices[i].setFadeTime(sec);
    }
  }

  void setRecPreSlewTime(size_t voice, float d) {
    voices[voice].setRecPreSlewTime(d);
  }
  void setRateSlewTime(size_t voice, float d) {
    voices[voice].setRateSlewTime(d);
  }
  void setMainWet(float val) { wetDrySlew.setTarget(val); }
  float getSavedPosition(size_t voice);

  void process(const float *inl, const float *inr, float *outl, float *outr,
               unsigned int numFrames);

 private:
  Voice voices[NUM_VOICES];
  float tapeSliceStart[NUM_VOICES] = {0};
  float tapeSliceEnd[NUM_VOICES] = {0};
  float loopStart[NUM_VOICES] = {0};
  float loopEnd[NUM_VOICES] = {0};
  float panning[NUM_VOICES] = {0};
  float panningL[NUM_VOICES] = {0};
  float panningR[NUM_VOICES] = {0};
  float levelsSet[NUM_VOICES] = {0};
  float levelsCur[NUM_VOICES] = {0};
  LinearRamp levelSlew[NUM_VOICES];
  LinearRamp wetDrySlew;
  float inputBus[NUM_VOICES] = {0};
  bool playing[NUM_VOICES] = {false};

  float *buf = nullptr;
  unsigned int bufFrames = 0;
  float sampleRate = 0;
};

}  // namespace softcut

#endif  // SOFTCUT_VOICES_H
