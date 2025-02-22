#include "TapeEmu.h"

#include <math.h>

TapeEmu::TapeEmu() {}

void TapeEmu::Init(float sample_rate) {
  follower = Follower(sample_rate);
  dc_input_l_ = 0;
  dc_output_l_ = 0;
  dc_input_r_ = 0;
  dc_output_r_ = 0;
  dc_gain_ = 0.99;
  bias_ = 0.5;
  pregain_ = 1.0;
  follow_ = 0;
}

void TapeEmu::SetBias(float bias) { bias_ = bias; }

void TapeEmu::SetPregain(float pregain) { pregain_ = pregain; }

void TapeEmu::Process(float **out, unsigned int numFrames) {
  for (size_t i = 0; i < numFrames; i++) {
    follow_ = follower.process(out[0][i]);

    // pregain
    out[0][i] *= pregain_;
    out[1][i] *= pregain_;

    // dc bias with follower
    out[0][i] = tanhf(out[0][i] + (follow_ * bias_));
    out[1][i] = tanhf(out[1][i] + (follow_ * bias_));

    // DC blocking
    float in = out[0][i];
    out[0][i] = in - dc_input_l_ + (dc_gain_ * dc_output_l_);
    dc_output_l_ = out[0][i];
    dc_input_l_ = in;
    in = out[1][i];
    out[1][i] = in - dc_input_r_ + (dc_gain_ * dc_output_r_);
    dc_output_r_ = out[1][i];
    dc_input_r_ = in;

    // extra tanh
    out[0][i] = tanhf(out[0][i]) * 0.5;
    out[1][i] = tanhf(out[1][i]) * 0.5;
  }
}
