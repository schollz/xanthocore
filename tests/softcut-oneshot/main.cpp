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
  float nonInterleavedBuffer[CONFIG_AUDIO_BLOCK_SIZE];
  float interleavedBuffer[2 * CONFIG_AUDIO_BLOCK_SIZE];
  float in[2][CONFIG_AUDIO_BLOCK_SIZE];
  float out[2][CONFIG_AUDIO_BLOCK_SIZE];
  // clear in
  memset(in, 0, sizeof(in));
  // clear out
  memset(out, 0, sizeof(out));
  // clear tape_linear_buffer
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));
  voices.init(tape_linear_buffer, MAX_SIZE, CONFIG_AUDIO_SAMPLE_RATE,
              CONFIG_AUDIO_BLOCK_SIZE);
  voices.setMainWet(1);
  voices.setLoopStart(0);
  voices.setLoopEnd(3);
  voices.setFadeTime(0.7);
  voices.setRecFlag(0, false);
  voices.setPlayFlag(0, true);
  voices.setPreLevel(0, 1);
  voices.setRecLevel(0, 0);
  voices.setRecPreSlewTime(0, 0.1);
  voices.setRateSlewTime(0, 0.0);
  voices.setRate(0, 1);
  voices.setDB(0, 0);
  voices.setRecOffset(0.005);

  int index = 48000 * 3;
  while (inFile.read(reinterpret_cast<char*>(nonInterleavedBuffer),
                     sizeof(nonInterleavedBuffer))) {
    // copy the input buffer to the tape_linear_buffer
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      tape_linear_buffer[index] = nonInterleavedBuffer[i];
      index++;
    }
  }
  inFile.close();

  // now for three seconds play the tape_linear_buffer
  float ratios[7] = {1.0,
                     1.1224620483089,
                     1.2599210498937,
                     1.3348398541685,
                     1.4983070768743,
                     1.6817928305039,
                     1.8877486253586};
  for (int j = 0; j < 10; j++) {
    voices.cutToPos(0, 0);
    float randomRatio = ratios[rand() % 7];
    voices.setRate(0, randomRatio);
    for (int i = 0; i < CONFIG_AUDIO_SAMPLE_RATE / CONFIG_AUDIO_BLOCK_SIZE;
         i++) {
      // in is empty
      // clear out
      memset(out, 0, sizeof(out));
      // clear in
      memset(in, 0, sizeof(in));
      const float* inPtrs[2] = {in[0], in[1]};
      float* outPtrs[2] = {out[0], out[1]};
      voices.process(inPtrs, outPtrs, CONFIG_AUDIO_BLOCK_SIZE);
      for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
        interleavedBuffer[i * 2] = out[0][i];
        interleavedBuffer[i * 2 + 1] = out[1][i];
      }
      outFile.write(reinterpret_cast<char*>(interleavedBuffer),
                    sizeof(interleavedBuffer));

      // print position of the first head
      std::cout << "Position of the first head: " << voices.getSavedPosition(0)
                << std::endl;
    }
  }

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
