#ifndef PYTOTUNE_PITCHSHIFTER_H
#define PYTOTUNE_PITCHSHIFTER_H
#include <functional>
#include <vector>

#include "pytotune/data-structures/windowing.h"

namespace p2t {
    class PitchShifter {
        Windowing windowing;
        float sampleRate;

    public:
        explicit PitchShifter(const Windowing windowing, const float sampleRate)
            : windowing(windowing), sampleRate(sampleRate) {
        }

    public:
        std::vector<float> run(const std::vector<float> &samples, const WindowedData<float> &pitchFactors,
                               bool limitToOctave = false) const;

        std::vector<float> run(const std::vector<float> &samples, float pitchFactor) const;

    private:
        std::vector<float> runWithClampedPitchFactors(const std::vector<float> &samples,
                                                      const WindowedData<float> &pitchFactors) const;
    };
} // namespace p2t
#endif  // PYTOTUNE_PITCHSHIFTER_H
