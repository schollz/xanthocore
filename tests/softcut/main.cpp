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

#include "../../lib/softcut/Voice.h"

#define MAX_SIZE (8388608)
#define AUDIO_SAMPLE_RATE 48000
#define NUM_VOICES 2
using namespace softcut;
Voice voices[NUM_VOICES];

// audioblock for softcut data
float tape_linear_buffer[MAX_SIZE * 2];

float processSample(float sample) {
  float out = 0;  // Output sum

  for (size_t i = 0; i < NUM_VOICES; i++) {
    float inBuffer[1] = {sample};  // Fresh copy for each voice
    float outBuffer[1] = {0};      // Fresh output buffer

    voices[i].processBlockMono(inBuffer, outBuffer, 1);
    out += outBuffer[0];  // Accumulate processed output
  }

  return sample + (out / (NUM_VOICES + 1));  // Average output
}

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
  // Initialize voices
  for (int i = 0; i < NUM_VOICES; i++) {
    voices[i].reset();
    voices[i].setBuffer(tape_linear_buffer, MAX_SIZE);
    voices[i].setSampleRate(AUDIO_SAMPLE_RATE);
    voices[i].setRate(1.0);
    voices[i].setLoopStart(1.0);
    voices[i].setLoopEnd(3.0);
    voices[i].setLoopFlag(true);
    voices[i].setFadeTime(0.1);
    voices[i].setRecLevel(0.0);
    voices[i].setPreLevel(0.0);
    if (i == 0) {
      voices[i].setRecFlag(true);
    }
    voices[i].setRecOnceFlag(false);
    voices[i].setPlayFlag(true);
    voices[i].cutToPos(1.0);
    voices[i].setRecPreSlewTime(0.5);
    voices[i].setRateSlewTime(1.5);
    voices[i].setRecOffset(-0.005);
  }

  float sample;
  int index = 0;
  while (inFile.read(reinterpret_cast<char*>(&sample), sizeof(float))) {
    if (index++ == 48000) {
      voices[0].setRecLevel(0);
      voices[0].setPreLevel(1.0);
    } else if (index == 1) {
      voices[0].setRecLevel(1.0);
      voices[0].setPreLevel(0.5);
    } else if (index == 96000 + 24000) {
      voices[0].setLoopEnd(1.5);
      voices[0].setRate(-0.25);
      voices[1].setRate(2.13);
    }
    float processedSample = processSample(sample);
    outFile.write(reinterpret_cast<char*>(&processedSample), sizeof(float));
  }

  inFile.close();
  outFile.close();

  std::cout << "Processing complete. Output written to " << outputFile
            << std::endl;
  return 0;
}
