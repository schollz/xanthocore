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

#include "../../lib/Follower.h"

#define CONFIG_AUDIO_SAMPLE_RATE 48000
#define CONFIG_AUDIO_BLOCK_SIZE 128

int main() {
  const char* inputFile = "output.raw";
  const char* outputFile = "follower.raw";

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

  float leftChannelIn[CONFIG_AUDIO_BLOCK_SIZE];

  Follower follower = Follower(CONFIG_AUDIO_SAMPLE_RATE, 0.0025, 0.02);

  while (inFile.read(reinterpret_cast<char*>(leftChannelIn),
                     sizeof(leftChannelIn))) {
    for (int i = 0; i < CONFIG_AUDIO_BLOCK_SIZE; i++) {
      leftChannelIn[i] = follower.process(leftChannelIn[i]);
    }
    outFile.write(reinterpret_cast<char*>(leftChannelIn),
                  sizeof(leftChannelIn));
  }

  inFile.close();
  outFile.close();

  std::cout << "Processing complete. Output written to " << outputFile
            << std::endl;
  return 0;
}
