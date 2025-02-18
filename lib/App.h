#ifndef APP_H
#define APP_H

#include <functional>
#include <iostream>
#include <map>
#include <optional>

#include "softcut/Voices.h"
class App {
 public:
  virtual ~App() = default;

  virtual void Init(float *tape, unsigned int numFrames, float sr,
                    float audioblockSize) = 0;
  virtual void Process(const float *const *in, float **out,
                       unsigned int numFrames) = 0;
  virtual void setMainWet(float val) = 0;

  virtual softcut::Voices &getVoices() = 0;

  // Callback management with enum-based keys
  std::map<int, std::function<void()>> callbacks;

  // Register a callback with an enum key
  void registerCallback(int key, std::function<void()> callback) {
    callbacks[key] = std::move(callback);
  }

 protected:
  // Derived classes can trigger callbacks safely
  void triggerCallback(int key) {
    auto it = callbacks.find(key);
    if (it != callbacks.end() && it->second) {
      it->second();  // Call the function
    } else {
      std::cerr << "Warning: No callback registered for event " << key
                << std::endl;
    }
  }
};

#endif
