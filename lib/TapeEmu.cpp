#include "TapeEmu.h"

#include <math.h>

TapeEmu::TapeEmu() {}

void TapeEmu::Init(float sample_rate) {
  follower = Follower(sample_rate);
  dc_input_l_ = 0;
  dc_output_l_ = 0;
  dc_input_r_ = 0;
  dc_output_r_ = 0;
  dc_gain_ = 0.995;
}

void TapeEmu::Process(const float *const *in, float **out,
                      unsigned int numFrames) {
  for (size_t i = 0; i < numFrames; i++) {
    // dc bias with follower
    float follow = follower.process(in[0][i]);
    out[0][i] = tanhf(in[0][i] + (follow / 2));
    out[1][i] = tanhf(in[1][i] + (follow / 2));

    // DC blocking
    out[0][i] = out[0][i] - dc_input_l_ + (dc_gain_ * dc_output_l_);
    dc_output_l_ = out[0][i];
    dc_input_l_ = out[0][i];
    out[1][i] = out[1][i] - dc_input_r_ + (dc_gain_ * dc_output_r_);
    dc_output_r_ = out[1][i];

    // extra tanh
    out[0][i] = tanhf(out[0][i]);
    out[1][i] = tanhf(out[1][i]);
  }
}
