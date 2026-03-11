#ifndef PYTOTUNE_PITCH_DETECTOR_H
#define PYTOTUNE_PITCH_DETECTOR_H
#include <stdexcept>

#include "../io/wav_file.h"
#include "pytotune/data-structures/windowing.h"

#define DEFAULT_THRESHOLD 0.05f
#define DEFAULT_DECIMATION_FACTOR 4

namespace p2t {
    /**
     * Frequency interval used to constrain pitch detection.
     */
    struct PitchRange {
        /// Lower bound in Hz.
        float min;
        /// Upper bound in Hz.
        float max;

        /**
         * Construct a validated pitch range.
         * @param minValue Lower bound in Hz (must be > 0).
         * @param maxValue Upper bound in Hz (must be greater than minValue).
         * @throws std::invalid_argument If bounds are invalid.
         */
        constexpr PitchRange(const float minValue, const float maxValue)
            : min(minValue), max(maxValue) {
            if (!(minValue > 0.0f && minValue < maxValue)) {
                throw std::invalid_argument("Invalid PitchRange");
            }
        }
    };

    /**
     * Common predefined frequency ranges for pitch detection.
     */
    namespace VoiceRanges {
        // Basic categories
        constexpr PitchRange HEARABLE = {20.f, 20000.f};
        constexpr PitchRange PIANO = {27.5f, 4186.f};

        constexpr PitchRange MAN = {82.41f, 523.25f}; // E2 - C5
        constexpr PitchRange WOMAN = {175.00f, 1046.50f}; // F3 - C6
        constexpr PitchRange HUMAN = {82.41f, 1046.50f}; // E2 - C6

        // Standard Singing Ranges
        constexpr PitchRange BASS = {82.41f, 261.63f}; // E2 - C4
        constexpr PitchRange BARITON = {98.00f, 392.00f}; // G2 - G4
        constexpr PitchRange TENOR = {130.81f, 523.25f}; // C3 - C5
        constexpr PitchRange ALTO = {175.00f, 698.46f}; // F3 - F5
        constexpr PitchRange SOPRANO = {261.63f, 1046.50f}; // C4 - C6

        constexpr PitchRange CAT_PURR = {25.f, 150.f};
    } // namespace VoiceRanges

    class YINPitchDetector {
    public:
        /**
         * Detect pitch per analysis window using the YIN algorithm.
         * @param audioBuffer Input audio samples.
         * @param pitchRange Minimum and maximum frequency to consider (Hz).
         * @param threshold Detection confidence threshold (typically between 0 and 1).
         * @param decimationFactor Downsampling factor used during preprocessing (default is 4).
         * @return Window-aligned detected pitch values in Hz.
         */
        [[nodiscard]] WindowedData<float> detectPitch(const WavData &audioBuffer, PitchRange pitchRange,
                                                      float threshold = DEFAULT_THRESHOLD, int decimationFactor = DEFAULT_DECIMATION_FACTOR) const;

        /**
         * Construct a detector with fixed analysis windowing.
         * @param windowing Window size and hop size used for all detection runs.
         */
        explicit YINPitchDetector(Windowing windowing);

    private:
        Windowing windowing;

        // Helper function only used internally
        static inline float sinc(float x);

        static std::vector<float> designLowpassFir(int taps, float cutoff);

        static std::vector<float> decimateZeroPhase(const std::vector<float> &input, int factor);

        static std::vector<float> convolve(const std::vector<float> &signal, const std::vector<float> &kernel);
    };
} // namespace p2t

#endif  // PYTOTUNE_PITCH_DETECTOR_H
