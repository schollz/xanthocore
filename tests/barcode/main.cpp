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

#include "../../lib/dsp/Oscillator.h"
#include "../../lib/softcut/Voices.h"

#define MAX_SIZE (8388608)
#define AUDIO_SAMPLE_RATE 48000
using namespace softcut;
unsigned int AUDIO_BLOCK_SIZE = 128;
Voices voices;

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
  memset(tape_linear_buffer, 0, sizeof(tape_linear_buffer));
  voices.init(tape_linear_buffer, MAX_SIZE, AUDIO_SAMPLE_RATE);

  int index = 0;
  float interleavedBuffer[AUDIO_BLOCK_SIZE * 2];
  float leftChannelIn[AUDIO_BLOCK_SIZE];
  float rightChannelIn[AUDIO_BLOCK_SIZE];
  float leftChannelOut[AUDIO_BLOCK_SIZE];
  float rightChannelOut[AUDIO_BLOCK_SIZE];
  Oscillator osc[4];
  for (int i = 0; i < 4; i++) {
    osc[i].Init(AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SIZE);
    osc[i].SetFreq(0.5 / (i + 1));
    osc[i].SetAmp(1.0);
  }
  osc[1].SetAmp(0.5);
  osc[1].SetAdd(0.5);

  while (inFile.read(reinterpret_cast<char*>(interleavedBuffer),
                     sizeof(interleavedBuffer))) {
    for (int i = 0; i < 4; i++) {
      voices.setPan(i, osc[i].Process());
      voices.setRate(i, osc[i + 1].Process());
      // std::cout << "osc[" << i << "]: " << osc[i].Value() << std::endl;
      break;
    }
    // deinterleave
    for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
      leftChannelIn[i] = interleavedBuffer[i * 2];
      rightChannelIn[i] = interleavedBuffer[i * 2 + 1];
      leftChannelOut[i] = 0;
      rightChannelOut[i] = 0;
      if (index == 0) {
        voices.setLoopStart(0, 0.0);
        voices.setLoopEnd(0, 1.0);
        voices.setLoopFlag(0, true);
        voices.setRecOnceFlag(0, false);
        voices.setRecFlag(0, true);
        voices.setPlayFlag(0, true);
        voices.setRecLevel(0, 1.0);
        voices.setPreLevel(0, 0.5);
      } else if (index == 2 * 48000) {
        voices.cutToPos(0, 0.5);
        voices.setRecLevel(0, 0);
        voices.setPreLevel(0, 1.0f);
        voices.setRate(0, 0.5);
      } else if (index == 4 * 48000) {
        voices.setRate(0, -0.5);
      } else if (index == 6 * 48000) {
        voices.setLoopEnd(0, 0.1);
      }
      index++;
    }

    voices.process(leftChannelIn, rightChannelIn, leftChannelOut,
                   rightChannelOut, AUDIO_BLOCK_SIZE);
    // interleave
    for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
      interleavedBuffer[i * 2] = leftChannelOut[i];
      interleavedBuffer[i * 2 + 1] = rightChannelOut[i];
    }

    outFile.write(reinterpret_cast<char*>(interleavedBuffer),
                  sizeof(interleavedBuffer));
  }

  inFile.close();
  outFile.close();

  std::cout << "Processing complete. Output written to " << outputFile
            << std::endl;
  return 0;
}
