#include "Barcode.h"

#include <time.h>

void Barcode::Init(float *tape, unsigned int numFrames, float sr,
                   float audioblockSize) {
  // seed randomness
  srand(time(NULL));
  xfadeSamples = (10 * audioblockSize);
  xfadeSeconds = static_cast<float>(xfadeSamples) / sr;

  voices.init(tape, numFrames, sr);
  // make all voices uses the same tape
  for (size_t i = 0; i < NUM_VOICES; i++) {
    voices.setTape(i, 0);
    voices.setPlayFlag(i, true);
    voices.setLoopFlag(i, true);
    voices.setRecFlag(i, false);
    voices.setRecOnceFlag(i, false);
    voices.setPreLevel(i, 1.0);
    voices.setRecLevel(i, 0.0);
    voices.setLoopStart(i, 0.0);
    voices.setLoopEnd(i, 2.0);
    voices.setFadeTime(i, xfadeSeconds);
    voices.setRecPreSlewTime(i, xfadeSeconds);
    voices.cutToPos(i, 0.0);
    voices.setLevel(i, 0);
    voices.setRate(i, rates[i]);
  }
  // set first voice to prime it to record
  voices.setRecFlag(0, true);
  setPlaying(false);
  setRecording(false);

  // initialize oscillators to random lfos with periods between 5 and 40
  // seconds
  for (size_t i = 0; i < NUM_VOICES; i++) {
    for (size_t j = 0; j < NUM_OSCILLATORS; j++) {
      osc[i][j].Init(sr / audioblockSize);
      float randomPeriod = 5 + static_cast<float>(rand()) / RAND_MAX * 35;
      osc[i][j].SetFreq(1.0 / randomPeriod);
      osc[i][j].SetAmp(1.0);
    }
  }
};

void Barcode::ToggleRecording(bool on) {
  if (on && !recording) {
    setPlaying(false);
    voices.setRecFlag(0, true);
    voices.setRate(0, 1.0);
    voices.setRecLevel(0, 0.0);
    voices.setPreLevel(0, 0.0);
    voices.cutToPos(0);
    voices.setLoopStart(0);
    voices.setLoopEnd(30);
    xfadeSamplesWait = xfadeSamples;
  } else if (!on && recording) {
    voices.setRecLevel(0, 0.0);
    xfadeSamplesWait = xfadeSamples;
  }
  setRecording(on);
}

void Barcode::Process(const float *inl, const float *inr, float *outl,
                      float *outr, unsigned int numFrames) {
  if (playing) {
    for (size_t i = 0; i < NUM_VOICES; i++) {
      for (size_t j = 0; j < NUM_OSCILLATORS; j++) {
        osc[i][j].Process();
      }
      voices.setPan(i, osc[i][LFO_PAN].Value());
      voices.setDB(i, dbs[i] + linlin(osc[i][LFO_AMP].Value(), -1.0f, 1.0f,
                                      -12.0, 0.0f));
      bool forward = osc[i][LFO_DIRECTION].Value() > 0;
      if (forward) {
        voices.setRate(i, rates[i]);
      } else {
        voices.setRate(i, -1 * rates[i]);
      }
      float start =
          linlin(osc[i][LFO_START].Value(), -1.0f, 1.0f, 0.0f, recordingStop);
      voices.setLoopStart(i, start);
      float length = linlin(osc[i][LFO_LENGTH].Value(), -1.0f, 1.0f, 0.1f,
                            recordingStop / 2);
      float end = start + length;
      if (end > recordingStop) {
        end = recordingStop;
      }
      voices.setLoopEnd(i, end);
      float pos = voices.getSavedPosition(i);
      //   print position, start, end, pan, of each voice
      if (pos > end || pos < start) {
        voices.cutToPos(i, start + (end - start) / 2.0f);
      }
    }
  }
  // fade out the voice and set it
  if (xfadeSamplesWait > 0) {
    xfadeSamplesWait -= numFrames;
    if (xfadeSamplesWait <= 0) {
      if (recording) {
        voices.cutToPos(0, 0.0);
        voices.setRecLevel(0, 1.0);
      } else {
        // turn off the recording
        voices.setRecFlag(0, false);
        // set the loop end point
        recordingStop = voices.getSavedPosition(0) - xfadeSeconds;
        // print recording stop
        voices.setLoopEnd(recordingStop);
        // set all voices to random positions
        for (size_t i = 0; i < NUM_VOICES; i++) {
          voices.cutToPos(
              i, static_cast<float>(rand()) / RAND_MAX * recordingStop);
        }
        setPlaying(true);
      }
    }
  }

  voices.process(inl, inr, outl, outr, numFrames);
}
