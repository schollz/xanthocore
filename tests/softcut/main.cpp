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

#include "../../lib/barcode/Barcode.h"
#include "../../lib/softcut/Voice.h"

#define MAX_SIZE (8388608)
#define AUDIO_SAMPLE_RATE 48000
using namespace softcut;
unsigned int AUDIO_BLOCK_SIZE = 128;
#define NUM_VOICES 2
Voice voices[NUM_VOICES];
Barcode barcode;

// audioblock for softcut data
float tape_linear_buffer[MAX_SIZE * 2];

void processBlock(float* leftChannel, float* rightChannel,
                  unsigned int blockSize) {
  voices[0].processBlockMono(leftChannel, leftChannel, blockSize);
  voices[1].processBlockMono(rightChannel, rightChannel, blockSize);
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
    voices[i].setLoopStart(1.0 + i * 10);
    voices[i].setLoopEnd(3.0 + i * 10);
    voices[i].setFadeTime(0.1);
    voices[i].setRecPreSlewTime(0.5);
    voices[i].setRateSlewTime(1.5);
    voices[i].setRecOffset(0.005);
    voices[i].setRecOnceFlag(false);
    voices[i].setLoopFlag(true);
    voices[i].setRecFlag(true);
    voices[i].setPlayFlag(true);
    voices[i].setRecLevel(1.0);
    voices[i].setPreLevel(0.5);
    voices[i].cutToPos(1.0 + (i * 10));
  }

  barcode.init(voices);

  int index = 0;
  float interleavedBuffer[AUDIO_BLOCK_SIZE * 2];
  float leftChannel[AUDIO_BLOCK_SIZE];
  float rightChannel[AUDIO_BLOCK_SIZE];

  while (inFile.read(reinterpret_cast<char*>(interleavedBuffer),
                     sizeof(interleavedBuffer))) {
    // deinterleave
    for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
      leftChannel[i] = interleavedBuffer[i * 2];
      rightChannel[i] = interleavedBuffer[i * 2 + 1];
      index++;
      if (index == 48000 * 2) {
        voices[0].setRecLevel(0.0);
        voices[1].setRecLevel(0.0);
        voices[0].setPreLevel(1.0);
        voices[1].setPreLevel(1.0);
        voices[0].setRate(0.5);
        voices[1].setRate(1.5);
      }
    }

    processBlock(leftChannel, rightChannel, AUDIO_BLOCK_SIZE);

    // interleave
    for (unsigned int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
      interleavedBuffer[i * 2] =
          (leftChannel[i] + interleavedBuffer[i * 2]) / 2;
      interleavedBuffer[i * 2 + 1] =
          (rightChannel[i] + interleavedBuffer[i * 2 + 1]) / 2;
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
