#ifndef PYTOTUNE_WAVREADER_H
#define PYTOTUNE_WAVREADER_H
#include <cstdint>
#include <string>
#include <vector>

namespace p2t {
/**
 * Represents decoded WAV audio data.
 */
struct WavData {
    /// Sample rate in Hz.
    uint32_t sampleRate = 0;

    /// Number of channels in the source audio.
    uint16_t numChannels = 0;

    /// Audio samples as normalized floats.
    std::vector<float> samples;
};

/**
 * Helper class for loading and storing WAV files.
 * For now, only one track (mono) is supported. All further channels are discarded.
 */
class WavFile {
   public:
    /**
     * Construct from decoded WAV data.
     * @param data WAV payload to store in this instance.
     */
    explicit WavFile(WavData data);

    /**
     * Get the decoded WAV data.
     * @return Reference to the stored `WavData`.
     */
    const WavData &data() const { return wavData; }

    /**
     * Load a WAV file from disk.
     * @param path Filesystem path to the WAV file.
     * @return WAV file instance holding decoded audio data.
     * @throws std::runtime_error On file I/O or unsupported/invalid format.
     */
    static WavFile load(const std::string &path);

    /**
     * Store audio to disk using default audio format.
     * @param path Destination WAV path.
     */
    void store(const std::string &path) const {
        store(path, 1);  // default to PCM
    }

    /**
     * Store audio to disk using an explicit WAV audio format code.
     * @param path Destination WAV path.
     * @param audioFormat WAV audio format code (e.g. PCM = 1).
     */
    void store(const std::string &path, uint16_t audioFormat) const;

   private:
    /**
     * Default constructor is private; use the static `load` factory.
     */
    WavFile() = default;

    /**
     * Read and parse a WAV file into internal storage.
     * @param path Path to the WAV file.
     * @throws std::runtime_error On parse or format errors.
     */
    void readFile(const std::string &path);

    /**
     * Storage for decoded WAV content.
     */
    WavData wavData;
};
}  // namespace p2t

#endif  // PYTOTUNE_WAVREADER_H
