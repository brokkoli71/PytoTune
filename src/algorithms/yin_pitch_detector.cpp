#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>

namespace p2t {
    WindowedData<float> YINPitchDetector::detect_pitch(const WavData &audio_buffer, const int f_min, const int f_max,
                                                       const float threshold = 0.4) const {
        if (f_min <= 0 || f_min >= f_max) {
            throw std::invalid_argument("f_min and f_max must be positive and f_min must be less than f_max.");
        }
        constexpr int decimation_factor = 4;
        const int tau_min = audio_buffer.sampleRate / f_max / decimation_factor;
        const int tau_max = audio_buffer.sampleRate / f_min / decimation_factor;

        if (windowing.windowSize < tau_max) {
            throw std::invalid_argument("Window size must be at least as large as the maximum tau. Consider increasing the window size or reducing f_min.");
        }
        std::vector<float> pitchValues;

        // todo: parallelize this loop (mutex for pitchValues or preallocate and write to index)
        for (int i = 0; i < audio_buffer.samples.size(); i += this->windowing.stride) {
            // Process each window
            const int window_end = std::min(i + this->windowing.windowSize, (int) audio_buffer.samples.size());

            // decimate
            // todo: more sophisticated approach
            std::vector<float> decimated_samples((window_end-i)/decimation_factor);
            for (int j = 0; j < decimated_samples.size(); ++j) {
                decimated_samples[j] = audio_buffer.samples[i + j * decimation_factor];
            }

            std::vector diff(tau_max + 1, 0.0f);
            for (int tau = tau_min; tau <= tau_max; ++tau) {
                float sum = 0.0f;
                for (int j = 0; j < decimated_samples.size() - tau; ++j) {
                    const float delta = decimated_samples[j] - decimated_samples[j + tau];
                    sum += delta * delta;
                }
                diff[tau] = sum;
            }

            // CMND (Cumulative Mean Normalized Difference Function)
            std::vector<float> cmnd(tau_max + 1, 0.0f);
            cmnd[0] = 1.0f; // Avoid division by zero
            float running_sum = 0.0f;
            for (int tau = 1; tau <= tau_max; ++tau)
            {
                running_sum += diff[tau];
                cmnd[tau] = diff[tau] * tau / running_sum;
            }

            // Find the pitch for this window based on the cmnd function and threshold
            int best_tau = tau_min;
            for (int tau = tau_min; tau <= tau_max; ++tau) {
                if (cmnd[tau] < cmnd[best_tau]) best_tau = tau; // as fallback
                if (cmnd[tau] < threshold) {
                    best_tau = tau;
                    break;
                }
            }
            best_tau *= decimation_factor; // compensate for decimation

            // std::cout << "diff[" << best_tau << "] = " << diff[best_tau] << std::endl;
            // std::cout << static_cast<float>(audio_buffer->sampleRate) / best_tau << std::endl;

            // quadratic interpolation to refine the estimate if best_tau is not at the boundaries
            // auto refined_tau = static_cast<float>(best_tau);
            // if (best_tau > 0 && best_tau < tau_max) {
            //     const auto left = diff[best_tau - 1];
            //     const auto center = diff[best_tau];
            //     const auto right = diff[best_tau + 1];
            //
            //     auto denom = 2 * (left + right - 2 * center);
            //     float correction = 0.0f;
            //     if (denom != 0.0f)
            //         correction = (left - right) / denom;
            //     refined_tau += correction;
            // }
            float pitch = static_cast<float>(audio_buffer.sampleRate) / best_tau;
            pitchValues.push_back(pitch);
        }
        return {
            this->windowing, pitchValues
        };
    }

    YINPitchDetector::YINPitchDetector(const Windowing windowing) : windowing(windowing) {
    }
}
