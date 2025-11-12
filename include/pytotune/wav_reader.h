//
// Created by hannes on 12/11/2025.
//

#ifndef PYTOTUNE_AUTOTUNE_H
#define PYTOTUNE_AUTOTUNE_H
#include <vector>

#include <string>
#include <cstdint>

struct WavData {
  uint32_t sampleRate = 0;
  uint16_t numChannels = 0;
  std::vector<float> samples;  // interleaved if stereo
};

class WavReader {
public:
  explicit WavReader(const std::string& path);
  const WavData& data() const { return wavData_; }

private:
  void readFile(const std::string& path);
  WavData wavData_;
};

#endif // PYTOTUNE_AUTOTUNE_H
