#ifndef Softcut_Voices_H
#define Softcut_Voices_H

#include "Voice.h"

#define NUM_VOICES 4

namespace softcut {
class Voices {
 public:
  Voices();
  void init(float *buf, unsigned int numFrames, float sampleRate);
  void setTape(size_t voice, size_t index);
  void setRate(size_t voice, float rate);
  void setDB(size_t voice, float db);
  void setLevel(size_t voice, float level);
  void setPan(size_t voice, float pan);
  void setLoopStart(size_t voice, float sec);
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
  void setRecPreSlewTime(size_t voice, float d) {
    voices[voice].setRecPreSlewTime(d);
  }
  void setRateSlewTime(size_t voice, float d) {
    voices[voice].setRateSlewTime(d);
  }
  float getSavedPosition(size_t voice);
  void process(const float *inl, const float *inr, float *outl, float *outr,
               unsigned int numFrames);

 private:
  Voice voices[NUM_VOICES];
  float tapeSliceStart[NUM_VOICES];
  float tapeSliceEnd[NUM_VOICES];
  float loopStart[NUM_VOICES];
  float loopEnd[NUM_VOICES];
  float panning[NUM_VOICES];
  float panningL[NUM_VOICES];
  float panningR[NUM_VOICES];
  float levels[NUM_VOICES];
  float inputBus[NUM_VOICES];
  bool playing[NUM_VOICES];

  float *buf;
  unsigned int bufFrames;
  float sampleRate;
};

}  // namespace softcut

#endif