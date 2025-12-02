

#ifndef PYTOTUNE_PITCHSHIFTER_H
#define PYTOTUNE_PITCHSHIFTER_H
#include <functional>
#include <vector>

namespace p2t {
    struct Windowing {
        int windowSize;
        int stride;
        float sampleRate;

        Windowing(int windowSize, int stride, float sampleRate)
            : windowSize(windowSize),
              stride(stride),
              sampleRate(sampleRate) {
        }

        Windowing(int windowSize, float overlapPercentage, float sampleRate)
            : windowSize(windowSize),
              stride(static_cast<float>(windowSize) * (1 - overlapPercentage)),
              sampleRate(sampleRate) {
        }

        int getOsamp() const {
            return windowSize / stride;
        }
    };

    template<typename T>
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

    class PitchShifter {
        Windowing windowing;

    public:
        PitchShifter(Windowing windowing) : windowing(windowing) {
        }

        std::vector<float> run(const std::vector<float> &samples, const WindowedData<float> &pitchFactors) const;

        std::vector<float> run(const std::vector<float> &samples, float pitchFactor) const;
    };
} // namespace p2t

#endif  // PYTOTUNE_PITCHSHIFTER_H
