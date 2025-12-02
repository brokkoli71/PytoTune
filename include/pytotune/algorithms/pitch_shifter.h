

#ifndef PYTOTUNE_PITCHSHIFTER_H
#define PYTOTUNE_PITCHSHIFTER_H
#include <functional>
#include <vector>

#include "pytotune/data-structures/windowing.h"

namespace p2t {
class PitchShifter {
    Windowing windowing;

   public:
    explicit PitchShifter(Windowing windowing) : windowing(windowing) {
    }

    std::vector<float> run(const std::vector<float> &samples, const WindowedData<float> &pitchFactors) const;

    std::vector<float> run(const std::vector<float> &samples, float pitchFactor) const;
};
}  // namespace p2t
#endif  // PYTOTUNE_PITCHSHIFTER_H
