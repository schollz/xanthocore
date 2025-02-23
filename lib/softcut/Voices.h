#ifndef SOFTCUT_VOICES_H
#define SOFTCUT_VOICES_H

#include "../Config.h"
#include "Utilities.h"
#include "Voice.h"

namespace softcut {

class Voices {
 public:
  Voices(){};

  void init(float *buf, unsigned int numFrames, float sampleRate,
            float audioblockSize);
  void setTape(size_t voice, size_t index);
  void setRate(size_t voice, float rate);
  void setDB(size_t voice, float db);
  void setLevel(size_t voice, float level);
  void setPan(size_t voice, float pan);
  void setLoopStart(size_t voice, float sec);
  void setLoopStart(float sec) {
    for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
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
    for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
      voices[i].setFadeTime(sec);
    }
  }
  void setRecOffset(float d) {
    for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
      voices[i].setRecOffset(d);
    }
  }

  void setRecPreSlewTime(size_t voice, float d) {
    voices[voice].setRecPreSlewTime(d);
  }
  void setRateSlewTime(size_t voice, float d) {
    voices[voice].setRateSlewTime(d);
  }
  void setMainWet(float val) {
    mainWet = val;
    mainDry = 1.0 - val;
  }
  float getSavedPosition(size_t voice);
  bool isPlaying(size_t voice) { return voices[voice].getPlayFlag(); }

  void process(const float *const *in, float **out, unsigned int numFrames);

 private:
  Voice voices[CONFIG_VOICE_NUM];
  float tapeSliceStart[CONFIG_VOICE_NUM] = {0};
  float tapeSliceEnd[CONFIG_VOICE_NUM] = {0};
  float loopStart[CONFIG_VOICE_NUM] = {0};
  float loopEnd[CONFIG_VOICE_NUM] = {0};
  float panning[CONFIG_VOICE_NUM] = {0};
  float panningL[CONFIG_VOICE_NUM] = {0};
  float panningR[CONFIG_VOICE_NUM] = {0};
  float levels[CONFIG_VOICE_NUM] = {0};
  float mainWet = 0.5;
  float mainDry = 0.5;
  float inputBus[CONFIG_VOICE_NUM] = {0};
  bool playing[CONFIG_VOICE_NUM] = {false};

  float *buf = nullptr;
  unsigned int bufFrames = 0;
  float sampleRate = 0;
};

}  // namespace softcut

#endif  // SOFTCUT_VOICES_H
