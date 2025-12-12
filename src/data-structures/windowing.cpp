#include "pytotune/data-structures/windowing.h"

namespace p2t {
    Windowing::Windowing(int windowSize, int stride, float sampleRate)
        : windowSize(windowSize),
          stride(stride),
          sampleRate(sampleRate) {
    }

    Windowing::Windowing(int windowSize, float overlapPercentage, float sampleRate)
        : windowSize(windowSize),
          stride(static_cast<int>((1.0f - overlapPercentage) * static_cast<float>(windowSize))),
          sampleRate(sampleRate) {
    }
}
