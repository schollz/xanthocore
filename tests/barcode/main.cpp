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
#include "../../lib/reverb2/Reverb2.h"

#define MAX_SIZE (8388608)
#define CONFIG_AUDIO_SAMPLE_RATE 48000
using namespace softcut;
unsigned int CONFIG_AUDIO_BLOCK_SIZE = 128;

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
  Barcode barcode;
  barcode.init(tape_linear_buffer, MAX_SIZE, CONFIG_AUDIO_SAMPLE_RATE,
               CONFIG_AUDIO_BLOCK_SIZE);

  Reverb2 reverb;
  reverb.SetFeedback(0.7);
  reverb.Init(CONFIG_AUDIO_SAMPLE_RATE);

  int index = 0;
  float interleavedBuffer[CONFIG_AUDIO_BLOCK_SIZE * 2];
  float leftChannelIn[CONFIG_AUDIO_BLOCK_SIZE];
  float rightChannelIn[CONFIG_AUDIO_BLOCK_SIZE];
  float leftChannelOut[CONFIG_AUDIO_BLOCK_SIZE];
  float rightChannelOut[CONFIG_AUDIO_BLOCK_SIZE];
  float reverbWet = 0.5;
  float reverbDry = 1.0 - reverbWet;

  while (inFile.read(reinterpret_cast<char*>(interleavedBuffer),
                     sizeof(interleavedBuffer))) {
    // deinterleave
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      leftChannelIn[i] = interleavedBuffer[i * 2];
      rightChannelIn[i] = interleavedBuffer[i * 2 + 1];
      leftChannelOut[i] = 0;
      rightChannelOut[i] = 0;
      index++;
      if (index == 1) {
        barcode.ToggleRecording(true);
      } else if (index == 5.5 * 48000) {
        barcode.ToggleRecording(false);
      }
    }

    barcode.process(leftChannelIn, rightChannelIn, leftChannelOut,
                    rightChannelOut, CONFIG_AUDIO_BLOCK_SIZE);

    // reverb
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      float outl, outr;
      reverb.Process(leftChannelOut[i], rightChannelOut[i], &outl, &outr);
      leftChannelOut[i] = reverbWet * outl + reverbDry * leftChannelOut[i];
      rightChannelOut[i] = reverbWet * outr + reverbDry * rightChannelOut[i];
    }

    // interleave
    for (unsigned int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      interleavedBuffer[i * 2] = (leftChannelOut[i]);
      interleavedBuffer[i * 2 + 1] = (rightChannelOut[i]);
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
