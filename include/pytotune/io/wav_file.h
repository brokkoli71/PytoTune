//
// Created by hannes on 12/11/2025.
//

#ifndef PYTOTUNE_WAVREADER_H
#define PYTOTUNE_WAVREADER_H
#include <vector>

#include <string>
#include <cstdint>

namespace p2t {
  struct WavData {
    uint32_t sampleRate = 0;
    uint16_t numChannels = 0;
    std::vector<float> samples; // interleaved if stereo
  };

  class WavFile {
  public:
    const WavData &data() const { return wavData_; }

    static WavFile load(const std::string &path);

  private:
    WavFile() = default;

    void readFile(const std::string &path);

    WavData wavData_;
  };
}

#endif // PYTOTUNE_WAVREADER_H
