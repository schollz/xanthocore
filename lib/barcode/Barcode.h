// a C++ class that holds softcut voices

#ifndef BARCODE_H
#define BARCODE_H

#include "../softcut/Voice.h"
using namespace softcut;

class Barcode {
 public:
  // make constructor with pointer to voices
  Barcode();

  void init(Voice *voices);

 private:
  Voice *voices;
};

#endif