

#ifndef PYTOTUNE_PITCHSHIFTER_H
#define PYTOTUNE_PITCHSHIFTER_H
#include <vector>

namespace p2t {
    struct Windowing {
        int windowSize;
        int stride;

        Windowing(int windowSize, int stride) : windowSize(windowSize), stride(stride) {
        }

        Windowing(int windowSize, float overlapPercentage)
            : windowSize(windowSize),
              stride(static_cast<float>(windowSize) * (1 - overlapPercentage)) {
        }

        int getOsamp() const {
            return windowSize / stride;
        }
    };

    class PitchShifter {
        Windowing windowing;
        float sampleRate;

    public:
        PitchShifter(Windowing windowing, float sampleRate) : windowing(windowing), sampleRate(sampleRate) {
        }

        std::vector<float> run(const std::vector<float> samples, float factor) const;
    };
} // namespace p2t

#endif  // PYTOTUNE_PITCHSHIFTER_H
