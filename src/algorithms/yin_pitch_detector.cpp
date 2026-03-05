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
    WindowedData<float> YINPitchDetector::detectPitch(const WavData &audioBuffer, const PitchRange pitchRange,
                                                      const float threshold = 0.4) const {
        constexpr int decimationFactor = 2; // or parameterize this

        std::vector<float> downsampledAudio =
                decimateZeroPhase(audioBuffer.samples, decimationFactor);


        const unsigned int downsampledFs =
                audioBuffer.sampleRate / decimationFactor;

        const unsigned int tauMin = downsampledFs / static_cast<int>(pitchRange.max);
        const unsigned int tauMax = downsampledFs / static_cast<int>(pitchRange.min);

        const unsigned int numWindows = (downsampledAudio.size() - this->windowing.windowSize) / this->windowing.
                                        stride + 1;
        std::vector<float> pitchValues(numWindows);
#pragma omp parallel for schedule(dynamic, 16)
        // prevent false sharing of pitchValues by using dynamic scheduling with small chunks
        for (int i = 0; i < numWindows; i++) {
            // Process each window
            const int windowStart = i * this->windowing.stride;
            const int windowEnd = std::min(windowStart + this->windowing.windowSize,
                                           static_cast<int>(downsampledAudio.size()));
            std::vector<float> windowSamples(
                downsampledAudio.begin() + windowStart,
                downsampledAudio.begin() + windowEnd);

            // --- Silence / low-energy detection ---
            float energy = 0.0f;
            for (float f: windowSamples) {
                energy += f * f;
            }
            energy /= windowSamples.size(); // mean square

            constexpr float silenceThreshold = 5e-5f;

            if (energy < silenceThreshold) {
                pitchValues[i] = 0.0f; // 0 Hz = unvoiced
                continue;
            }

            std::vector<float> diff(tauMax + 1);
            for (unsigned int tau = tauMin; tau <= tauMax; ++tau) {
                // computation of sum_{j=0}^{N-1} (x[j] - x[j+tau])^2
                size_t k = 0;
                const float *aPtr = windowSamples.data();
                const float *bPtr = aPtr + tau;
                float sum = 0.0f;
                const size_t n = (windowSamples.size() > tau) ? (windowSamples.size() - tau) : 0;
                if (n > 0) {
# if USE_HWY
                    // Vectorized computation using Highway
                    ScalableTag<float> d;
                    auto vecAcc = Set(d, 0.0f);
                    const size_t lanes = Lanes(d);

                    for (; k + lanes <= n; k += lanes) {
                        const auto va = Load(d, aPtr + k);
                        const auto vb = Load(d, bPtr + k);
                        const auto diffv = va - vb;
                        vecAcc = vecAcc + diffv * diffv;
                    }

                    // Horizontal reduce SIMD accumulator into scalar sum
                    std::vector<float> tmp(lanes);
                    Store(vecAcc, d, tmp.data());
                    for (size_t i = 0; i < lanes; ++i) sum += tmp[i];
# endif
                    // Collect remaining elements
                    for (; k < n; ++k) {
                        const float delta = aPtr[k] - bPtr[k];
                        sum += delta * delta;
                    }
                }
                diff[tau] = sum;
            }

            // CMND (Cumulative Mean Normalized Difference Function)
            std::vector<float> cmnd(tauMax + 1);
            float runningSum = 1.0f;
            for (int tau = 1; tau <= tauMax; ++tau) {
                runningSum += diff[tau];
                cmnd[tau] = diff[tau] * static_cast<float>(tau) / runningSum;
            }

            // Find the pitch for this window based on the cmnd function and threshold
            unsigned int bestTau = 0;

            // Search for first local minimum below threshold
            for (unsigned int tau = tauMin + 1; tau < tauMax - 1; ++tau) {
                if (cmnd[tau] < threshold &&
                    cmnd[tau] < cmnd[tau - 1] &&
                    cmnd[tau] <= cmnd[tau + 1]) {
                    bestTau = tau;
                    break;
                }
            }

            // Fallback: global minimum in range if nothing passed threshold
            if (bestTau == 0) {
                bestTau = tauMin;
                for (int tau = tauMin + 1; tau <= tauMax; ++tau) {
                    if (cmnd[tau] < cmnd[bestTau]) {
                        bestTau = tau;
                    }
                }
            }

            // Quadratic interpolation around best_tau (on CMND)
            auto refinedTau = static_cast<float>(bestTau);

            if (bestTau > tauMin && bestTau < tauMax) {
                const float left = cmnd[bestTau - 1];
                const float center = cmnd[bestTau];
                const float right = cmnd[bestTau + 1];

                const float denom = 2.0f * (left - 2.0f * center + right);

                if (std::abs(denom) > 1e-12f) {
                    const float delta = (left - right) / denom;
                    refinedTau += delta;
                }
            }
            pitchValues[i] = static_cast<float>(downsampledFs) / refinedTau;
        }

        // smooth the pitch values by removing octave jumps down
        for (size_t i = 1; i < pitchValues.size(); ++i) {
            float octaveJumpThreshold = pitchValues[i - 1] / 1.5f;
            // if the pitch drops by more than a perfect fifth, it's likely an octave jump
            if (pitchValues[i] < octaveJumpThreshold) {
                pitchValues[i] *= 2.0f; // assume it's an octave jump and correct it
            }
        }
        // smooth by median filter with window size 5
        std::vector<float> smoothedPitches = pitchValues;
        const int medianWindow = 5;
        for (int i = 0; i < pitchValues.size(); ++i) {
            std::vector<float> window;
            for (int j = -medianWindow / 2; j <= medianWindow / 2; ++j) {
                if (j + i >= 0 && i + j < pitchValues.size()) {
                    window.push_back(pitchValues[i + j]);
                }
            }
            std::sort(window.begin(), window.end());
            smoothedPitches[i] = window[window.size() / 2]; // median
        }

        // Get original windowing
        const unsigned int uncompressedNumWindows = audioBuffer.samples.size() / this->windowing.stride;
        std::vector<float> uncompressedPitchValues(uncompressedNumWindows, 0.0f);

        for (int i = 0; i < uncompressedNumWindows; ++i) {
            int j = i / decimationFactor;
            if (j >= smoothedPitches.size() - 1) {
                uncompressedPitchValues[i] = smoothedPitches[smoothedPitches.size() - 1];
            } else {
                // Linear Interpolation
                const float start = smoothedPitches[j];
                const float end = start == 0 ? 0 : (smoothedPitches[j + 1] == 0 ? start : smoothedPitches[j + 1]);
                const float frac = static_cast<float>(i % decimationFactor) / decimationFactor;
                uncompressedPitchValues[i] = (1 - frac) * start + frac * end;
            }
        }

        return {
            this->windowing, uncompressedPitchValues
        };
    }

    YINPitchDetector::YINPitchDetector(const Windowing windowing) : windowing(windowing) {
    }

    inline float YINPitchDetector::sinc(const float x) {
        if (std::abs(x) < 1e-8f) return 1.0f;
        return static_cast<float>(std::sin(M_PI * x) / (M_PI * x));
    }

    // Design low-pass FIR using windowed-sinc (Hamming window)
    std::vector<float> YINPitchDetector::designLowpassFir(const int taps, const float cutoff) {
        std::vector<float> h(taps);
        const int m = taps - 1;

        for (int n = 0; n < taps; ++n) {
            float centered = n - m / 2.0f;

            float ideal = 2.0f * cutoff * sinc(2.0f * cutoff * centered);

            float window = 0.54f - 0.46f * std::cos(2.0f * M_PI * n / m);

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
        const unsigned int signalSize = signal.size();
        const unsigned int kernalSize = kernel.size();
        std::vector<float> output(signalSize, 0.0f);

        for (int n = 0; n < signalSize; ++n) {
            float acc = 0.0f;
            for (int k = 0; k < kernalSize; ++k) {
                int idx = n - k;
                if (idx >= 0 && idx < signalSize)
                    acc += kernel[k] * signal[idx];
            }
            output[n] = acc;
        }

        return output;
    }

    // Zero-phase decimation
    std::vector<float> YINPitchDetector::decimateZeroPhase(const std::vector<float> &input, const int factor) {
        if (factor <= 1)
            return input;

        if (input.empty())
            return {};

        // FIR parameters
        constexpr int taps = 101; // increase for sharper cutoff
        const float cutoff = 0.5f / static_cast<float>(factor); // normalized cutoff

        auto fir = designLowpassFir(taps, cutoff);

        // Forward filter
        auto forward = convolve(input, fir);

        // Reverse
        std::vector<float> reversed(forward.rbegin(), forward.rend());

        // Backward filter
        auto backward = convolve(reversed, fir);

        // Reverse again → zero phase
        std::vector<float> filtered(backward.rbegin(), backward.rend());

        // Downsample using HWY-aligned memory
        const size_t outputSize = filtered.size() / factor;

        // Allocate HWY-aligned memory (aligned to vector width for optimal SIMD access)
        float * HWY_RESTRICT alignedOutput = static_cast<float *>(
            hwy::AllocateAlignedBytes(outputSize * sizeof(float), nullptr, nullptr));

        if (!alignedOutput) {
            throw std::runtime_error("Failed to allocate aligned memory for decimation");
        }

        // Downsample into aligned buffer
        size_t outIdx = 0;
        for (size_t i = 0; i < filtered.size(); i += factor) {
            alignedOutput[outIdx++] = filtered[i];
        }

        // Copy to std::vector and free aligned memory
        std::vector<float> output(alignedOutput, alignedOutput + outputSize);
        hwy::FreeAlignedBytes(alignedOutput, nullptr, nullptr);

        return output;
    }
}
