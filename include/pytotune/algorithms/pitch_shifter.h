#ifndef PYTOTUNE_PITCHSHIFTER_H
#define PYTOTUNE_PITCHSHIFTER_H
#include <functional>
#include <vector>

#include "pytotune/data-structures/windowing.h"

namespace p2t {
    /**
     * Time-domain pitch shifting for windowed audio.
     *
     * Applies multiplicative pitch factors to audio samples using the configured
     * windowing and sample rate.
     */
    class PitchShifter {
        Windowing windowing;
        float sampleRate;

    public:
        /**
         * Construct a pitch shifter with fixed processing parameters.
         * @param windowing Window size and hop size used during processing.
         * @param sampleRate Audio sample rate in Hz.
         */
        explicit PitchShifter(Windowing windowing, float sampleRate) : windowing(windowing), sampleRate(sampleRate) {
        }

        /**
         * Shift pitch with per-window factors.
         * @param samples Input mono audio samples.
         * @param pitchFactors Multiplicative pitch factors for each analysis window.
         * @return Pitch-shifted audio samples.
         */
        std::vector<float> run(const std::vector<float> &samples, const WindowedData<float> &pitchFactors) const;

        /**
         * Shift pitch with one global factor.
         * @param samples Input mono audio samples.
         * @param pitchFactor Global multiplicative pitch factor.
         * @return Pitch-shifted audio samples.
         */
        std::vector<float> run(const std::vector<float> &samples, float pitchFactor) const;
    };
} // namespace p2t
#endif  // PYTOTUNE_PITCHSHIFTER_H
