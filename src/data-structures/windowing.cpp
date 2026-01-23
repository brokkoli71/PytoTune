#include "pytotune/data-structures/windowing.h"

namespace p2t {
    Windowing::Windowing(int windowSize, int stride)
        : windowSize(windowSize),
          stride(stride) {
    }

    Windowing::Windowing(int windowSize, float overlapPercentage)
        : windowSize(windowSize),
          stride(static_cast<int>((1.0f - overlapPercentage) * static_cast<float>(windowSize))) {
    }
}
