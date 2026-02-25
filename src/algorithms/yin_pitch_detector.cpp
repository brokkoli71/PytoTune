#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>

#include <cmath>

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
#pragma omp parallel for
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

            std::vector diff(tau_max + 1, 0.0f);
            for (unsigned int tau = tau_min; tau <= tau_max; ++tau) {
                float sum = 0.0f;
                for (int j = 0; j < window_samples.size() - tau; ++j) {
                    const float delta = window_samples[j] - window_samples[j + tau];
                    sum += delta * delta;
                }
                diff[tau] = sum;
            }

            // CMND (Cumulative Mean Normalized Difference Function)
            std::vector<float> cmnd(tau_max + 1, 0.0f);
            float running_sum = 1.0f;
            for (int tau = 1; tau <= tau_max; ++tau) {
                running_sum += diff[tau];
                cmnd[tau] = diff[tau] * static_cast<float>(tau) / running_sum;
            }

            // Find the pitch for this window based on the cmnd function and threshold
            unsigned int best_tau = 0;

            // 1) Search for first local minimum below threshold
            for (unsigned int tau = tau_min + 1; tau < tau_max - 1; ++tau) {
                if (cmnd[tau] < threshold &&
                    cmnd[tau] < cmnd[tau - 1] &&
                    cmnd[tau] <= cmnd[tau + 1]) {
                    best_tau = tau;
                    break;
                }
            }

            // 2) Fallback: global minimum in range if nothing passed threshold
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
            pitchValues[i] = static_cast<float>(downsampled_fs) / static_cast<float>(best_tau);
        }

        // Get original windowing
        const unsigned int uncompressed_num_windows = audio_buffer.samples.size() / this->windowing.stride;
        std::vector<float> uncompressed_pitch_values(uncompressed_num_windows, 0.0f);

        for (int i = 0; i < uncompressed_num_windows; ++i) {
            int j = i / decimation_factor;
            if (j >= pitchValues.size() - 1) {
                uncompressed_pitch_values[i] = pitchValues[pitchValues.size() - 1];
            } else {
                // Linear Interpolation
                const float start = pitchValues[j];
                const float end = start == 0 ? 0 : (pitchValues[j + 1] == 0 ? start : pitchValues[j + 1]);
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

        // Downsample
        std::vector<float> output;
        output.reserve(filtered.size() / factor);

        for (std::size_t i = 0; i < filtered.size(); i += factor)
            output.push_back(filtered[i]);

        return output;
    }
}
