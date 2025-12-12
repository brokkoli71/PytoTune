//
// Created by Moritz Seppelt on 02.12.25.
//

#ifndef PYTOTUNE_WINDOWING_H
#define PYTOTUNE_WINDOWING_H
#include <functional>
#include <vector>

namespace p2t {
struct Windowing {
    int windowSize;
    int stride;
    float sampleRate;

    Windowing(int windowSize, int stride, float sampleRate);

    Windowing(int windowSize, float overlapPercentage, float sampleRate);

    int getOsamp() const {
        return windowSize / stride;
    }
};

template <typename T>
struct WindowedData {
    Windowing windowing;
    std::vector<T> data;

    explicit WindowedData(Windowing windowing) : windowing(windowing) {
    }

    WindowedData(Windowing windowing, std::vector<T> data) : windowing(windowing), data(data) {
    }

    static WindowedData<T> fromLambda(Windowing windowing, int dataSize, std::function<T(float)> lambda) {
        std::vector<T> data(dataSize);
        for (int i = 0; i < dataSize; i++) {
            data[i] = lambda(static_cast<float>(i * windowing.stride) / windowing.sampleRate);
        }
        return {windowing, data};
    }
};
}  // namespace p2t

#endif  // PYTOTUNE_WINDOWING_H
