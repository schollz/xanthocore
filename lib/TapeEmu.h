#ifndef LIB_TAPEEMU_H
#define LIB_TAPEEMU_H

#include "Follower.h"

class TapeEmu {
 public:
  TapeEmu();
  void Init(float sample_rate);
  void Process(const float *const *in, float **out, unsigned int numFrames);

 private:
  float dc_input_l_, dc_output_l_, dc_gain_;
  float dc_input_r_, dc_output_r_;
  Follower follower;
};
#endif