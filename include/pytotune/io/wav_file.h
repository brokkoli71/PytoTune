/**
 * @file wav_file.h
 * @brief Lightweight WAV reader declarations.
 *
 * Provides WavData and WavFile for loading simple WAV files (16-bit PCM or 32-bit float)
 * into a normalized floating-point sample buffer.
 *
 * Example:
 *   auto wav = p2t::WavFile::load("sound.wav");
 *   const auto &d = wav.data();
 *   // d.sampleRate, d.numChannels, d.samples
 */

#ifndef PYTOTUNE_WAVREADER_H
#define PYTOTUNE_WAVREADER_H
#include <vector>

#include <string>
#include <cstdint>

namespace p2t {
  /**
   * @struct WavData
   * @brief Represents decoded WAV audio data.
   *
   * The samples vector contains floating point samples normalized to the range [-1.0, 1.0].
   * If numChannels > 1 the samples are interleaved (channel0, channel1, ...). TODO: might want to change that later.
   */
  struct WavData {
    /**
     * @brief Sample rate in Hertz (Hz).
     *
     * Zero indicates no data or uninitialized.
     */
    uint32_t sampleRate = 0; // in Hz

    /**
     * @brief Number of channels (1 = mono, 2 = stereo, ...).
     * For now, only mono is supported.
     */
    uint16_t numChannels = 0;

    /**
     * @brief Interleaved audio samples as floats in [-1.0, 1.0].
     *
     * For multi-channel audio the layout is:
     *   sample0_chan0, sample0_chan1, ..., sample1_chan0, sample1_chan1, ...
     */
    std::vector<float> samples; // interleaved if stereo
  };

  /**
   * @class WavFile
   * @brief Simple helper to load WAV files into a WavData structure.
   *
   * Use WavFile::load(path) to create an instance. The loader currently supports
   * 16-bit PCM and 32-bit float WAV formats. Loading failures throw std::runtime_error.
   */
  class WavFile {
  public:
    /**
     * @brief Get the loaded WAV data.
     * @return const WavData& Reference to the internal WavData.
     */
    const WavData &data() const { return wavData_; }

    /**
     * @brief Load a WAV file from disk.
     * @param path Filesystem path to the WAV file.
     * @return WavFile Instance holding the decoded data.
     * @throws std::runtime_error on file I/O or format errors.
     *
     * For now, stereo files will only load first channel and ignore others.
     */
    static WavFile load(const std::string &path);

  private:
    /**
     * @brief Default constructor is private; use the static load() factory.
     */
    WavFile() = default;

    /**
     * @brief Read and parse the given WAV file into wavData_.
     * @param path Path to the WAV file to read.
     * @throws std::runtime_error on failure to open, parse or unsupported formats.
     */
    void readFile(const std::string &path);

    /**
     * @brief Storage for the decoded WAV content.
     */
    WavData wavData_;
  };
}

#endif // PYTOTUNE_WAVREADER_H
