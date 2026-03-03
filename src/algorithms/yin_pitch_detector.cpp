#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>

#include <cmath>
#include "hwy/highway.h"
#include "hwy/aligned_allocator.h"

using namespace hwy::HWY_NAMESPACE;
# define USE_HWY 1
namespace p2t {
    WindowedData<float> YINPitchDetector::detect_pitch(const WavData &audio_buffer, const PitchRange pitch_range,
                                                       const float threshold = 0.4) const {
        constexpr int decimation_factor = 2; // or parameterize this

        std::vector<float> downsampled_audio =
            decimate_zero_phase(audio_buffer.samples, decimation_factor);


        const unsigned int downsampled_fs =
                audio_buffer.sampleRate / decimation_factor;

        const unsigned int tau_min = downsampled_fs / static_cast<int>(pitch_range.max);
        const unsigned int tau_max = downsampled_fs / static_cast<int>(pitch_range.min);

        const unsigned int num_windows = (downsampled_audio.size() - this->windowing.windowSize) / this->windowing.
                                         stride + 1;
        std::vector<float> pitchValues(num_windows);
#pragma omp parallel for schedule(dynamic, 16) // prevent false sharing of pitchValues by using dynamic scheduling with small chunks
        for (int i = 0; i < num_windows; i++) {
            // Process each window
            const int window_start = i * this->windowing.stride;
            const int window_end = std::min(window_start + this->windowing.windowSize,
                                            static_cast<int>(downsampled_audio.size()));
            std::vector<float> window_samples(
                downsampled_audio.begin() + window_start,
                downsampled_audio.begin() + window_end);

            // --- Silence / low-energy detection ---
            float energy = 0.0f;
            for (float f: window_samples) {
                energy += f * f;
            }
            energy /= window_samples.size(); // mean square

            constexpr float silence_threshold = 1e-6f;

            if (energy < silence_threshold) {
                pitchValues[i] = 0.0f; // 0 Hz = unvoiced
                continue;
            }

            std::vector<float> diff(tau_max + 1);
            for (unsigned int tau = tau_min; tau <= tau_max; ++tau) {
                // computation of sum_{j=0}^{N-1} (x[j] - x[j+tau])^2
                size_t k = 0;
                const float *a_ptr = window_samples.data();
                const float *b_ptr = a_ptr + tau;
                float sum = 0.0f;
                const size_t N = (window_samples.size() > tau) ? (window_samples.size() - tau) : 0;
                if (N > 0) {
# if USE_HWY
                    // Vectorized computation using Highway
                    ScalableTag<float> d;
                    auto vec_acc = Set(d, 0.0f);
                    const size_t lanes = Lanes(d);

                    for (; k + lanes <= N; k += lanes) {
                        const auto va = Load(d, a_ptr + k);
                        const auto vb = Load(d, b_ptr + k);
                        const auto diffv = va - vb;
                        vec_acc = vec_acc + diffv * diffv;
                    }

                    // Horizontal reduce SIMD accumulator into scalar sum
                    std::vector<float> tmp(lanes);
                    Store(vec_acc, d, tmp.data());
                    for (size_t i = 0; i < lanes; ++i) sum += tmp[i];
# endif
                    // Collect remaining elements
                    for (; k < N; ++k) {
                        const float delta = a_ptr[k] - b_ptr[k];
                        sum += delta * delta;
                    }
                }
                diff[tau] = sum;
            }

            // CMND (Cumulative Mean Normalized Difference Function)
            std::vector<float> cmnd(tau_max + 1);
            float running_sum = 1.0f;
            for (int tau = 1; tau <= tau_max; ++tau) {
                running_sum += diff[tau];
                cmnd[tau] = diff[tau] * static_cast<float>(tau) / running_sum;
            }

            // Find the pitch for this window based on the cmnd function and threshold
            unsigned int best_tau = 0;

            // Search for first local minimum below threshold
            for (unsigned int tau = tau_min + 1; tau < tau_max - 1; ++tau) {
                if (cmnd[tau] < threshold &&
                    cmnd[tau] < cmnd[tau - 1] &&
                    cmnd[tau] <= cmnd[tau + 1]) {
                    best_tau = tau;
                    break;
                }
            }

            // Fallback: global minimum in range if nothing passed threshold
            if (best_tau == 0) {
                best_tau = tau_min;
                for (int tau = tau_min + 1; tau <= tau_max; ++tau) {
                    if (cmnd[tau] < cmnd[best_tau]) {
                        best_tau = tau;
                    }
                }
            }

            // Quadratic interpolation around best_tau (on CMND)
            auto refined_tau = static_cast<float>(best_tau);

            if (best_tau > tau_min && best_tau < tau_max) {
                const float left = cmnd[best_tau - 1];
                const float center = cmnd[best_tau];
                const float right = cmnd[best_tau + 1];

                const float denom = 2.0f * (left - 2.0f * center + right);

                if (std::abs(denom) > 1e-12f) {
                    const float delta = (left - right) / denom;
                    refined_tau += delta;
                }
            }
            pitchValues[i] = static_cast<float>(downsampled_fs) / refined_tau;
        }

        // smooth the pitch values by removing octave jumps down
        for (size_t i = 1; i < pitchValues.size(); ++i) {
            float octave_jump_threshold = pitchValues[i-1] / 1.5f; // if the pitch drops by more than a perfect fifth, it's likely an octave jump
            if (pitchValues[i] < octave_jump_threshold) {
                pitchValues[i] *= 2.0f; // assume it's an octave jump and correct it
            }
        }
        // smooth by median filter with window size 5
        std::vector<float> smoothed_pitches = pitchValues;
        const int median_window = 5;
        for (int i = 0; i < pitchValues.size(); ++i) {
            std::vector<float> window;
            for (int j = -median_window/2; j <= median_window/2; ++j) {
                if (j + i >= 0 && i + j < pitchValues.size()) {
                    window.push_back(pitchValues[i + j]);
                }
            }
            std::sort(window.begin(), window.end());
            smoothed_pitches[i] = window[window.size() / 2]; // median
        }

        // Get original windowing
        const unsigned int uncompressed_num_windows = audio_buffer.samples.size() / this->windowing.stride;
        std::vector<float> uncompressed_pitch_values(uncompressed_num_windows, 0.0f);

        for (int i = 0; i < uncompressed_num_windows; ++i) {
            int j = i / decimation_factor;
            if (j >= smoothed_pitches.size() - 1) {
                uncompressed_pitch_values[i] = smoothed_pitches[smoothed_pitches.size() - 1];
            } else {
                // Linear Interpolation
                const float start = smoothed_pitches[j];
                const float end = start == 0 ? 0 : (smoothed_pitches[j + 1] == 0 ? start : smoothed_pitches[j + 1]);
                const float frac = static_cast<float>(i % decimation_factor) / decimation_factor;
                uncompressed_pitch_values[i] = (1 - frac) * start + frac * end;
            }
        }

        return {
            this->windowing, uncompressed_pitch_values
        };
    }

    YINPitchDetector::YINPitchDetector(const Windowing windowing) : windowing(windowing) {
    }

    inline float YINPitchDetector::sinc(const float x) {
        if (std::abs(x) < 1e-8f) return 1.0f;
        return static_cast<float>(std::sin(M_PI * x) / (M_PI * x));
    }

    // Design low-pass FIR using windowed-sinc (Hamming window)
    std::vector<float> YINPitchDetector::design_lowpass_fir(const int taps, const float cutoff) {
        std::vector<float> h(taps);
        const int M = taps - 1;

        for (int n = 0; n < taps; ++n) {
            float centered = n - M / 2.0f;

            float ideal = 2.0f * cutoff * sinc(2.0f * cutoff * centered);

            float window = 0.54f - 0.46f * std::cos(2.0f * M_PI * n / M);

            h[n] = ideal * window;
        }

        // Normalize for unity DC gain
        float sum = 0.0f;
        for (float v: h) sum += v;
        for (float &v: h) v /= sum;

        return h;
    }

    // Convolution (valid for FIR filtering)
    std::vector<float> YINPitchDetector::convolve(
        const std::vector<float> &signal,
        const std::vector<float> &kernel) {
        const unsigned int N = signal.size();
        const unsigned int K = kernel.size();
        std::vector<float> output(N, 0.0f);

        for (int n = 0; n < N; ++n) {
            float acc = 0.0f;
            for (int k = 0; k < K; ++k) {
                int idx = n - k;
                if (idx >= 0 && idx < N)
                    acc += kernel[k] * signal[idx];
            }
            output[n] = acc;
        }

        return output;
    }

    // Zero-phase decimation
    std::vector<float> YINPitchDetector::decimate_zero_phase(const std::vector<float> &input, const int factor) {
        if (factor <= 1)
            return input;

        if (input.empty())
            return {};

        // FIR parameters
        constexpr int taps = 101; // increase for sharper cutoff
        const float cutoff = 0.5f / static_cast<float>(factor); // normalized cutoff

        auto fir = design_lowpass_fir(taps, cutoff);

        // Forward filter
        auto forward = convolve(input, fir);

        // Reverse
        std::vector<float> reversed(forward.rbegin(), forward.rend());

        // Backward filter
        auto backward = convolve(reversed, fir);

        // Reverse again → zero phase
        std::vector<float> filtered(backward.rbegin(), backward.rend());

        // Downsample using HWY-aligned memory
        const size_t output_size = filtered.size() / factor;
        
        // Allocate HWY-aligned memory (aligned to vector width for optimal SIMD access)
        float* HWY_RESTRICT aligned_output = static_cast<float*>(
            hwy::AllocateAlignedBytes(output_size * sizeof(float), nullptr, nullptr));
        
        if (!aligned_output) {
            throw std::runtime_error("Failed to allocate aligned memory for decimation");
        }

        // Downsample into aligned buffer
        size_t out_idx = 0;
        for (size_t i = 0; i < filtered.size(); i += factor) {
            aligned_output[out_idx++] = filtered[i];
        }

        // Copy to std::vector and free aligned memory
        std::vector<float> output(aligned_output, aligned_output + output_size);
        hwy::FreeAlignedBytes(aligned_output, nullptr, nullptr);

        return output;
    }
}
