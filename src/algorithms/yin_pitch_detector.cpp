#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#elif defined(__AVX2__)
#include <immintrin.h>
#endif

#define OPTIMIZATION_LVL 2 // 0: none, 1: loop unrolling, 2: SIMD

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
                int j = i;
                const int limit = window_end - tau;
# if OPTIMIZATION_LVL >= 2 && (defined(__ARM_NEON) || defined(__ARM_NEON__))
                // TODO do we want this anyway?
                // Optimization for Apple Silicon / ARM64
                float32x4_t v_sum = vdupq_n_f32(0.0f);
                for (; j <= limit - 4; j += 4) {
                    float32x4_t v_a = vld1q_f32(&audio_buffer.samples[j]);
                    float32x4_t v_b = vld1q_f32(&audio_buffer.samples[j + tau]);
                    float32x4_t v_diff = vsubq_f32(v_a, v_b);
                    // Multiply-accumulate: sum += diff * diff
                    v_sum = vmlaq_f32(v_sum, v_diff, v_diff);
                }
                // Horizontal sum of the vector
                sum += vaddvq_f32(v_sum);

# elif OPTIMIZATION_LVL >= 2 && defined(__AVX2__)
                // Optimization for modern x86_64
                __m256 v_sum = _mm256_setzero_ps();
                for (; j <= limit - 8; j += 8) {
                    // TODO allign loads?
                    __m256 v_a = _mm256_loadu_ps(&audio_buffer.samples[j]);
                    __m256 v_b = _mm256_loadu_ps(&audio_buffer.samples[j + tau]);
                    __m256 v_diff = _mm256_sub_ps(v_a, v_b);
                    // Multiply-accumulate: sum += diff * diff (Requires FMA support, usually present with AVX2)
                    // If FMA isn't available, use: v_sum = _mm256_add_ps(v_sum, _mm256_mul_ps(v_diff, v_diff));
                    v_sum = _mm256_fmadd_ps(v_diff, v_diff, v_sum);
                }
                // Horizontal sum for AVX2
                float temp[8];
                _mm256_storeu_ps(temp, v_sum);
                for (float t: temp) sum += t;
# elif OPTIMIZATION_LVL >= 1
                // loop unrolling
                float sum0 = 0.0f, sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
                while (j < limit - 3) {
                    float d0 = audio_buffer.samples[j] - audio_buffer.samples[j + tau];
                    float d1 = audio_buffer.samples[j + 1] - audio_buffer.samples[j + 1 + tau];
                    float d2 = audio_buffer.samples[j + 2] - audio_buffer.samples[j + 2 + tau];
                    float d3 = audio_buffer.samples[j + 3] - audio_buffer.samples[j + 3 + tau];

                    sum0 += d0 * d0;
                    sum1 += d1 * d1;
                    sum2 += d2 * d2;
                    sum3 += d3 * d3;
                    j += 4;
                }
                sum += (sum0 + sum1 + sum2 + sum3);


#endif
                // scalar fallback for remaining samples or if OPTIMIZATION_LVL == 0
                for (; j < limit; ++j) {
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
