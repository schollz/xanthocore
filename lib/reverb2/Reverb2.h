/*
Copyright (c) 2023 Electrosmith, Corp, Sean Costello, Istvan Varga, Paul
Batchelor

Use of this source code is governed by the LGPL V2.1
license that can be found in the LICENSE file or at
https://opensource.org/license/lgpl-2-1/
*/

#pragma once
#ifndef DSYSP_REVERB2_H
#define DSYSP_REVERB2_H

#define DSY_REVERB2_MAX_SIZE (262144)

namespace daisysp {
/**Delay line for internal reverb use
 */
typedef struct {
  int write_pos;         /**< write position */
  int buffer_size;       /**< buffer size */
  int read_pos;          /**< read position */
  int read_pos_frac;     /**< fractional component of read pos */
  int read_pos_frac_inc; /**< increment for fractional */
  int dummy;             /**<  dummy var */
  int seed_val;          /**< randseed */
  int rand_line_cnt;     /**< number of random lines */
  float filter_state;    /**< state of filter */
  float *buf;            /**< buffer ptr */
} Reverb2Dl;

/** Stereo Reverb */
class Reverb2 {
 public:
  Reverb2() {}
  ~Reverb2() {}
  /** Initializes the reverb module, and sets the sample_rate at which the
     Process function will be called. Returns 0 if all good, or 1 if it runs out
     of delay times exceed maximum allowed.
  */
  int Init(float sample_rate, float *buf);

  /** Process the input through the reverb, and updates values of out1, and out2
   * with the new processed signal.
   */
  int Process(const float &in1, const float &in2, float *out1, float *out2);

  void Process(const float *const *in, float **out, size_t num);

  void SetWet(float wet);
  /** controls the reverb time. reverb tail becomes infinite when set to 1.0
      \param fb - sets reverb time. range: 0.0 to 1.0
  */
  inline void SetFeedback(const float &fb) { feedback_ = fb; }
  /** controls the internal dampening filter's cutoff frequency.
      \param freq - low pass frequency. range: 0.0 to sample_rate / 2
  */
  inline void SetLpFreq(const float &freq) { lpfreq_ = freq; }

 private:
  void NextRandomLineseg(Reverb2Dl *lp, int n);
  int InitDelayLine(Reverb2Dl *lp, int n);
  float feedback_, lpfreq_;
  float i_sample_rate_, i_pitch_mod_, i_skip_init_;
  float sample_rate_;
  float damp_fact_;
  float prv_lpfreq_;
  int init_done_;
  float *aux_;
  float wet_;
  float dry_;
  Reverb2Dl delay_lines_[8];
};

}  // namespace daisysp
#endif
