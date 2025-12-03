#include "../include/pytotune/yin_pitch_detector.h"

#include <algorithm>
#include <iostream>
#include <ostream>

namespace p2t
{
    PitchDetection YINPitchDetector::detect_pitch(const WavData* audio_buffer, const int f_min, const int f_max, const float threshold) const
    {
        if (f_min <= 0 || f_min >= f_max)
        {
            throw std::invalid_argument("f_min and f_max must be positive and f_min must be less than f_max.");
        }
        const int tau_min = audio_buffer->sampleRate / f_max;
        const int tau_max = audio_buffer->sampleRate / f_min;

        PitchDetection result;
        result.window_size = this->window_size;
        result.window_overlap = this->window_overlap;
        result.audio_buffer = audio_buffer;
        result.pitch_values = std::vector<float>();

        for (size_t i = 0; i < audio_buffer->samples.size(); i += this->window_size-this->window_overlap)
        {
            // Process each window
            const size_t window_end = std::min(i + this->window_size, audio_buffer->samples.size());
            std::vector diff(tau_max + 1, 0.0f);

            for (int tau = tau_min; tau <= tau_max; ++tau)
            {
                float sum = 0.0f;
                for (size_t j = i; j < window_end - tau; ++j)
                {
                    const float delta = audio_buffer->samples[j] - audio_buffer->samples[j + tau];
                    sum += delta * delta;
                }
                diff[tau] = sum;
            }

            // // CMND (Cumulative Mean Normalized Difference Function)
            // std::vector<float> cmnd(tau_max + 1, 0.0f);
            // cmnd[0] = 1.0f; // Avoid division by zero
            // float running_sum = 0.0f;
            // for (int tau = 1; tau <= tau_max; ++tau)
            // {
            //     running_sum += diff[tau];
            //     cmnd[tau] = diff[tau] * tau / running_sum;
            // }

            // Find the pitch for this window based on the diff function and threshold
            int best_tau = tau_min;
            for (int tau = tau_min; tau <= tau_max; ++tau)
            {
                if (diff[tau] < diff[best_tau]) best_tau = tau; // as fallback
                if (diff[tau] < threshold)
                {
                    best_tau = tau;
                    break;
                }
            }
            std::cout << "diff[" << best_tau << "] = " << diff[best_tau] << std::endl;
            std::cout << static_cast<float>(audio_buffer->sampleRate) / best_tau << std::endl;

            // quadratic interpolation to refine the estimate if best_tau is not at the boundaries
            auto refined_tau = static_cast<float>(best_tau);
            if (best_tau > 0 && best_tau < tau_max)
            {
                const auto left = diff[best_tau - 1];
                const auto center = diff[best_tau];
                const auto right = diff[best_tau + 1];

                auto denom = 2 * (left + right - 2 * center);
                float correction = 0.0f;
                if (denom != 0.0f)
                    correction = (left - right) / denom;
                refined_tau += correction;
            }
            float pitch = static_cast<float>(audio_buffer->sampleRate) / refined_tau;
            result.pitch_values.push_back(pitch);
        }
        return result;
    }

    YINPitchDetector::YINPitchDetector(int window_size, int window_overlap)
    {
        this->window_size = window_size;
        this->window_overlap = window_overlap;
    }

}
