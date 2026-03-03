#ifndef PYTOTUNE_PITCH_DETECTOR_H
#define PYTOTUNE_PITCH_DETECTOR_H
#include <stdexcept>

#include "../io/wav_file.h"
#include "pytotune/data-structures/windowing.h"

namespace p2t {
    struct PitchRange {
        float min;
        float max;

        constexpr PitchRange(const float minValue, const float maxValue)
            : min(minValue), max(maxValue) {
            if (!(minValue > 0.0f && minValue < maxValue)) {
                throw std::invalid_argument("Invalid PitchRange");
            }
        }
    };

    namespace VoiceRanges {
        // Basic categories
        constexpr PitchRange HEARABLE = {20.f, 20000.f};
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
    }

    class YINPitchDetector {
    public:
        /**
         * Detects the pitch of the given audio buffer using the YIN algorithm with preset parameters.
         * @param audio_buffer The audio buffer containing the WavData.
         * @param pitch_range Minimum and maximum frequency to consider (in Hz).
         * @param threshold Threshold for pitch detection confidence (between 0 and 1).
         * @return A PitchDetection struct containing the detected pitch information.
         */
        [[nodiscard]] WindowedData<float> detect_pitch(const WavData &audio_buffer, PitchRange pitch_range,
                                                       float threshold) const;

        /**
        * Constructor of the PitchDetector class.
        * @param windowing The windowing of all future pitch detection runs
        */
        explicit YINPitchDetector(Windowing windowing);

    private:
        Windowing windowing;

        // Helper function only used internally
        static inline float sinc(float x);

        static std::vector<float> design_lowpass_fir(int taps, float cutoff);

        static std::vector<float> decimate_zero_phase(const std::vector<float> &input, int factor);

        static std::vector<float> convolve(const std::vector<float> &signal, const std::vector<float> &kernel);
    };
}


#endif //PYTOTUNE_PITCH_DETECTOR_H
