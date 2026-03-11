#ifndef PYTOTUNE_WINDOWING_H
#define PYTOTUNE_WINDOWING_H
#include <functional>
#include <vector>

namespace p2t {
    /**
     * Window configuration for frame-based audio processing.
     */
    struct Windowing {
        /// Number of samples per analysis window.
        int windowSize;
        /// Hop size in samples between consecutive windows.
        int stride;

        /**
         * Construct windowing using explicit window size and stride.
         * @param windowSize Number of samples per analysis window.
         * @param stride Hop size in samples between consecutive windows.
         */
        Windowing(int windowSize, int stride);

        /**
         * Construct windowing from window size and overlap percentage.
         * @param windowSize Number of samples per analysis window.
         * @param overlapPercentage Overlap fraction between 0 and 1.
         */
        Windowing(int windowSize, float overlapPercentage);

        /**
         * Get oversampling factor used by phase-vocoder style processing.
         * @return Oversampling factor, computed as windowSize / stride.
         */
        int getOsamp() const {
            return windowSize / stride;
        }
    };

    /**
     * Data series aligned to a specific windowing configuration.
     * @tparam T Element type stored per window.
     */
    template<typename T>
    struct WindowedData {
        Windowing windowing;
        std::vector<T> data;

        /**
         * Construct an empty windowed data container.
         * @param windowing Windowing metadata for this container.
         */
        explicit WindowedData(Windowing windowing) : windowing(windowing) {
        }

        /**
         * Construct windowed data from an existing value vector.
         * @param windowing Windowing metadata for this container.
         * @param data Values associated with each analysis window.
         */
        WindowedData(Windowing windowing, std::vector<T> data) : windowing(windowing), data(data) {
        }

        /**
         * Generate windowed data by sampling a function at window start times.
         * @param windowing Windowing metadata for the generated data.
         * @param dataSize Number of window values to generate.
         * @param sampleRate Audio sample rate in Hz.
         * @param lambda Function mapping time in seconds to a value.
         * @return Generated windowed data.
         */
        static WindowedData<T> fromLambda(Windowing windowing, int dataSize, float sampleRate,
                                          std::function<T(float)> lambda) {
            std::vector<T> data(dataSize);
            for (int i = 0; i < dataSize; i++) {
                data[i] = lambda(static_cast<float>(i * windowing.stride) / sampleRate);
            }
            return {windowing, data};
        }
    };
} // namespace p2t

#endif  // PYTOTUNE_WINDOWING_H
