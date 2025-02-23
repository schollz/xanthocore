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

#include "../../lib/Config.h"
#include "../../lib/softcut/Voices.h"
using namespace softcut;

Voices voices;
#define MAX_SIZE (8388608)

// audioblock for softcut data
float tape_linear_buffer[MAX_SIZE * 2];

int main() {
  const char* inputFile = "output.raw";
  const char* outputFile = "processed.raw";

  std::ifstream inFile(inputFile, std::ios::binary);
  if (!inFile) {
    std::cerr << "Error: Cannot open " << inputFile << " for reading."
              << std::endl;
    return 1;
  }

  std::ofstream outFile(outputFile, std::ios::binary);
  if (!outFile) {
    std::cerr << "Error: Cannot open " << outputFile << " for writing."
              << std::endl;
    return 1;
  }

  // clear tape_linear_buffer
  int index = 0;
  float interleavedBuffer[CONFIG_AUDIO_BLOCK_SIZE * 2];
  float in[2][CONFIG_AUDIO_BLOCK_SIZE];
  float out[2][CONFIG_AUDIO_BLOCK_SIZE];
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));
  voices.init(tape_linear_buffer, MAX_SIZE, CONFIG_AUDIO_SAMPLE_RATE,
              CONFIG_AUDIO_BLOCK_SIZE);
  voices.setMainWet(1);
  voices.setLoopStart(0);
  voices.setLoopEnd(30);
  voices.setFadeTime(1);
  voices.setRecFlag(0, false);
  voices.setPlayFlag(0, false);
  voices.setPreLevel(0, 1);
  voices.setRecLevel(0, 0);
  voices.setRecPreSlewTime(0, 0.1);
  voices.setRateSlewTime(0, 0.1);
  voices.setRate(0, 1);
  voices.setDB(0, 0);
  voices.setRecOffset(0.005);

  while (inFile.read(reinterpret_cast<char*>(interleavedBuffer),
                     sizeof(interleavedBuffer))) {
    // deinterleave
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      in[0][i] = interleavedBuffer[i * 2];
      in[1][i] = interleavedBuffer[i * 2 + 1];
      out[0][i] = 0;
      out[1][i] = 0;
      index++;
      if (index == 1) {
        voices.setLoopStart(0, voices.getSavedPosition(0));
        voices.setLoopEnd(0, voices.getSavedPosition(0) + 2);
        voices.setRecLevel(0, 1);
        voices.setPlayFlag(0, true);
        voices.setRecOnceFlag(0, true);
      }
    }

    const float* inPtrs[2] = {in[0], in[1]};
    float* outPtrs[2] = {out[0], out[1]};
    voices.process(inPtrs, outPtrs, CONFIG_AUDIO_BLOCK_SIZE);

    // interleave
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      interleavedBuffer[i * 2] = out[0][i];
      interleavedBuffer[i * 2 + 1] = out[1][i];
    }

    outFile.write(reinterpret_cast<char*>(interleavedBuffer),
                  sizeof(interleavedBuffer));
  }

  //

  inFile.close();
  outFile.close();

  // write the tape_linear_buffer to a file using a writer
  const char* tapeFile = "tape.raw";
  std::ofstream tapeOut(tapeFile, std::ios::binary);
  if (!tapeOut) {
    std::cerr << "Error: Cannot open " << tapeFile << " for writing."
              << std::endl;
    return 1;
  }
  tapeOut.write(reinterpret_cast<char*>(tape_linear_buffer),
                sizeof(tape_linear_buffer));
  tapeOut.close();

  std::cout << "Processing complete. Output written to " << outputFile
            << std::endl;
  return 0;
}
