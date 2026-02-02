#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>

namespace p2t {
    WindowedData<float> YINPitchDetector::detect_pitch(const WavData &audio_buffer, const int f_min, const int f_max,
                                                       const float threshold) const {
        if (f_min <= 0 || f_min >= f_max) {
            throw std::invalid_argument("f_min and f_max must be positive and f_min must be less than f_max.");
        }
        const int tau_min = audio_buffer.sampleRate / f_max;
        const int tau_max = audio_buffer.sampleRate / f_min;

        std::vector<float> pitchValues;

        for (int i = 0; i < audio_buffer.samples.size(); i += this->windowing.stride) {
            const int window_end = std::min(i + this->windowing.windowSize, (int) audio_buffer.samples.size());

            std::vector diff(tau_max + 1, 0.0f);
            for (int tau = 0; tau <= tau_max; ++tau) { // Calculate for low tau for CMND
                float sum = 0.0f;
                for (int j = i; j < window_end - tau; ++j) {
                    const float delta = audio_buffer.samples[j] - audio_buffer.samples[j + tau];
                    sum += delta * delta;
                }
                diff[tau] = sum;
            }

            // Cumulative Mean Normalized Difference Function (CMND)
            std::vector cmnd(tau_max + 1, 0.0f);
            cmnd[0] = 1.0f;
            float running_sum = 0.0f;
            for (int tau = 1; tau <= tau_max; ++tau) {
                running_sum += diff[tau];
                if (running_sum == 0) {
                    cmnd[tau] = 1.0f;
                } else {
                    cmnd[tau] = diff[tau] * tau / running_sum;
                }
            }

            int best_tau = -1;
            for (int tau = tau_min; tau <= tau_max; ++tau) {
                if (cmnd[tau] < threshold) {
                    // Determine if this is a local minimum
                    while (tau + 1 <= tau_max && cmnd[tau + 1] < cmnd[tau]) {
                        tau++;
                    }
                    best_tau = tau;
                    break;
                }
            }
            // Fallback: Global minimum if no threshold passed
            if (best_tau == -1) {
                best_tau = tau_min;
                for (int tau = tau_min + 1; tau <= tau_max; ++tau) {
                    if (cmnd[tau] < cmnd[best_tau]) {
                        best_tau = tau;
                    }
                }
            }

            // std::cout << "diff[" << best_tau << "] = " << diff[best_tau] << std::endl;
            // std::cout << static_cast<float>(audio_buffer->sampleRate) / best_tau << std::endl;

            // quadratic interpolation to refine the estimate if best_tau is not at the boundaries
            auto refined_tau = static_cast<float>(best_tau);
            if (best_tau > tau_min && best_tau < tau_max) {
                const auto left = cmnd[best_tau - 1];
                const auto center = cmnd[best_tau];
                const auto right = cmnd[best_tau + 1];

                auto denom = 2 * (left + right - 2 * center);
                if (std::abs(denom) > 1e-6) {
                    refined_tau += (left - right) / denom;
                }
            }

            float pitch = (refined_tau > 0) ? (static_cast<float>(audio_buffer.sampleRate) / refined_tau) : 0.0f;
            pitchValues.push_back(pitch);
        }
        return {
            this->windowing, pitchValues
        };
    }

    YINPitchDetector::YINPitchDetector(const Windowing windowing) : windowing(windowing) {
    }
}
