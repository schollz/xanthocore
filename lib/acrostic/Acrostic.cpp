#include "Acrostic.h"

#include <time.h>

void Acrostic::Init(float *tape, unsigned int numFrames, float sr,
                    float audioblockSize) {
  // seed randomness
  srand(time(NULL));

  bpm_ = 120.0f;

  voices.init(tape, numFrames, sr, audioblockSize);
  // make all voices uses the same tape
  for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
    voices.setTape(i, i);
    voices.setLoopFlag(i, true);
    voices.setPlayFlag(i, false);
    voices.setRecFlag(i, false);
    voices.setRecOnceFlag(i, false);
    voices.setPreLevel(i, 0.0);
    voices.setRecLevel(i, 0.0);
    voices.setLoopStart(i, 0.0);
    voices.setLoopEnd(i, 30.0);
    voices.setFadeTime(i, 0.0);
    voices.setRecPreSlewTime(i, 0.1);
    voices.setLevel(i, 1.0);
    voices.setRate(i, 1.0);
    voices.cutToPos(i, 0.0);
  }
  // process the voices a few times to get their positions set
  for (size_t i = 0; i < CONFIG_AUDIO_BLOCK_SIZE * 10; i++) {
    float dummy1[2] = {0.0, 0.0};
    float dummy2[2] = {0.0, 0.0};
    float *in[2] = {dummy1, dummy2};
    float *out[2] = {dummy1, dummy2};
    voices.process(in, out, 1);
  }
  voices.setFadeTime(0.5);

  // initialize oscillators to random lfos with periods between 5 and 40
  // seconds
  for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
    for (size_t j = 0; j < NUM_OSCILLATORS; j++) {
      osc[i][j].Init(sr / audioblockSize);
      float randomPeriod = 5 + static_cast<float>(rand()) / RAND_MAX * 35;
      osc[i][j].SetFreq(1.0 / randomPeriod);
      osc[i][j].SetAmp(1.0);
    }
  }
};

void Acrostic::RecordLoop(uint8_t voice) {
  voices.cutToPos(voice, 0.0f);
  voices.setLoopEnd(voice, 60.0f / bpm_ * 16);
  voices.setRecLevel(voice, 1.0);
  voices.setPlayFlag(voice, true);
  voices.setRecOnceFlag(voice, true);
}

void Acrostic::Process(const float *const *in, float **out,
                       unsigned int numFrames) {
  for (size_t i = 0; i < CONFIG_VOICE_NUM; i++) {
    if (!voices.isPlaying(i)) {
      continue;
    }
    for (size_t j = 0; j < NUM_OSCILLATORS; j++) {
      osc[i][j].Process();
    }
    voices.setPan(i, osc[i][LFO_PAN].Value());
    voices.setDB(i, linlin(osc[i][LFO_AMP].Value(), -1.0f, 1.0f, -6.0, 6.0f));
  }

  voices.process(in, out, numFrames);
}
