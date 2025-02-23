#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "../../lib/Chords.h"

int main() {
  Chords chords;
  for (uint8_t beat = 0; beat < 16; beat++) {
    std::cout << chords.GetNote(beat) << std::endl;
  }
  return 0;
}
