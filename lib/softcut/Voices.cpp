
#include "Voices.h"

#include <cstring>

#define VOICES_TAPE_BUFFER 3.0f
using namespace softcut;

Voices::Voices() {}

void Voices::init(float *tape, unsigned int numFrames, float sr) {
  // erase tape
  memset(tape, 0, numFrames * sizeof(float));
  bufFrames = numFrames;
  sampleRate = sr;

  for (size_t i = 0; i < NUM_VOICES; i++) {
    loopStart[i] = 0;
    loopEnd[i] = 2;
    panning[i] = 0.1;  // this will be set
    inputBus[i] = 0;
    voices[i].reset();
    voices[i].setBuffer(tape, numFrames);
    voices[i].setSampleRate(sampleRate);
    voices[i].setRate(1.0);
    voices[i].setFadeTime(0.1);
    voices[i].setRecLevel(0.0);
    voices[i].setPreLevel(1.0);
    voices[i].setRecPreSlewTime(0.5);
    voices[i].setRateSlewTime(0.5);
    voices[i].setRecOnceFlag(false);
    voices[i].setLoopFlag(true);
    voices[i].setRecFlag(false);
    voices[i].setPlayFlag(false);
    setTape(i, i);
    setPan(i, 0.0f);
    setLevel(i, 1.0f);
    setLoopStart(i, 0.0f);
    setLoopEnd(i, 2.0f);
    cutToPos(i, 0.0f);
  }
}

void Voices::setTape(size_t voice, size_t index) {
  float totalSeconds = (float)bufFrames / (float)sampleRate;
  totalSeconds = totalSeconds - (VOICES_TAPE_BUFFER * 2);
  tapeSliceStart[voice] =
      VOICES_TAPE_BUFFER + (float)index / (float)NUM_VOICES * totalSeconds;
  tapeSliceEnd[voice] = (float)(index + 1) / (float)NUM_VOICES * totalSeconds;
  setLoopStart(voice, loopStart[voice]);
  setLoopEnd(voice, loopEnd[voice]);
}

void Voices::setRate(size_t voice, float rate) { voices[voice].setRate(rate); }

void Voices::setLoopStart(size_t voice, float sec) {
  loopStart[voice] = sec;
  voices[voice].setLoopStart(loopStart[voice] + tapeSliceStart[voice]);
}

void Voices::setLoopEnd(size_t voice, float sec) {
  if (sec > tapeSliceEnd[voice] - tapeSliceStart[voice]) {
    sec = tapeSliceEnd[voice] - tapeSliceStart[voice];
  }
  loopEnd[voice] = sec;
  voices[voice].setLoopEnd(loopEnd[voice] + tapeSliceStart[voice]);
}

void Voices::setLoopEnd(float sec) {
  for (size_t i = 0; i < NUM_VOICES; i++) {
    setLoopEnd(i, sec);
  }
}

void Voices::cutToPos(size_t voice, float sec) {
  voices[voice].cutToPos(sec + tapeSliceStart[voice]);
}

void Voices::cutToPos(float sec) {
  for (size_t i = 0; i < NUM_VOICES; i++) {
    cutToPos(i, sec);
  }
}

float Voices::getSavedPosition(size_t voice) {
  return voices[voice].getSavedPosition() - tapeSliceStart[voice];
}

void Voices::setLoopFlag(size_t voice, bool val) {
  voices[voice].setLoopFlag(val);
}

void Voices::setRecFlag(size_t voice, bool val) {
  voices[voice].setRecFlag(val);
}

void Voices::setRecOnceFlag(size_t voice, bool val) {
  voices[voice].setRecOnceFlag(val);
}

void Voices::setPlayFlag(size_t voice, bool val) {
  voices[voice].setPlayFlag(val);
}

void Voices::setPan(size_t voice, float pan) {
  pan = fclamp(pan, -1.0f, 1.0f);
  if (pan == panning[voice]) {
    return;
  }
  panning[voice] = pan;
  panningL[voice] = sinf(0.25f * M_PI * (1.0f - pan));
  panningR[voice] = cosf(0.25f * M_PI * (1.0f - pan));
}

void Voices::setLevel(size_t voice, float level) {
  level = fclamp(level, 0.0f, 2.0f);
  levels[voice] = level / (float)NUM_VOICES;
}

void Voices::setDB(size_t voice, float db) { setLevel(voice, dbamp(db)); }

void Voices::setPreLevel(size_t voice, float val) {
  voices[voice].setPreLevel(val);
}

void Voices::setRecLevel(size_t voice, float val) {
  voices[voice].setRecLevel(val);
}

void Voices::process(const float *inl, const float *inr, float *outl,
                     float *outr, unsigned int numFrames) {
  for (size_t i = 0; i < NUM_VOICES; i++) {
    if (voices[i].getPlayFlag() == false) {
      continue;
    }
    float output[numFrames];
    memset(output, 0, numFrames * sizeof(float));
    if (inputBus[i] == 0) {
      voices[i].processBlockMono(inl, output, numFrames);
    } else {
      voices[i].processBlockMono(inr, output, numFrames);
    }
    for (size_t j = 0; j < numFrames; j++) {
      output[j] *= levels[i];
      outl[j] += output[j] * panningL[i];
      outr[j] += output[j] * panningR[i];
    }
  }
  // wet/dry mix
  if (mainDry > 0) {
    for (size_t i = 0; i < numFrames; i++) {
      outl[i] = mainWet * outl[i] + mainDry * inl[i];
      outr[i] = mainWet * outr[i] + mainDry * inr[i];
    }
  }
}