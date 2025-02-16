#ifndef APP_H
#define APP_H

#include "softcut/Voices.h"
class App {
 public:
  virtual ~App() = default;

  virtual void Init(float *tape, unsigned int numFrames, float sr,
                    float audioblockSize) = 0;
  virtual void Process(const float *inl, const float *inr, float *outl,
                       float *outr, unsigned int numFrames) = 0;
  virtual void setMainWet(float val) = 0;

  virtual softcut::Voices &getVoices() = 0;
};

#endif
