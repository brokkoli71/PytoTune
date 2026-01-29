#include "pytotune/data-structures/windowing.h"

namespace p2t {
    Windowing::Windowing(int windowSize, int stride)
        : windowSize(windowSize),
          stride(stride) {
    }

    Windowing::Windowing(int windowSize, float overlapPercentage)
        : windowSize(windowSize),
          stride(windowSize - static_cast<int>(overlapPercentage * static_cast<float>(windowSize))) {
    }
}
