#ifndef PYTOTUNE_API_H
#define PYTOTUNE_API_H

#include <string>
#include <vector>

#include "data-structures/scale.h"

namespace p2t {
    /**
     * Tune a wav file using a midi file as reference.
     * @param wav_path Path to the input wav file
     * @param midi_path Path to the input midi file
     * @param out_path Path to the output wav file
     */
    void tune_to_midi(const std::string &wav_path, const std::string &midi_path, const std::string &out_path);

    /**
     * Tune a wav file to a musical scale.
     * @param wav_path Path to the input wav file
     * @param scale The scale object
     * @param out_path Path to the output wav file
     */
    void tune_to_scale(const std::string &wav_path, const Scale &scale, const std::string &out_path);

    /**
     * Tune raw audio samples to a musical scale (live mode).
     * @param samples Input audio samples as floats in [-1.0, 1.0]
     * @param sample_rate Sample rate in Hz
     * @param scale The scale object
     * @return Processed audio samples
     */
    std::vector<float> tune_array_to_scale(const std::vector<float> &samples, unsigned int sample_rate, const Scale &scale);

    /**
     * Tune raw audio samples to a target note frequency (live mode).
     * @param samples Input audio samples as floats in [-1.0, 1.0]
     * @param sample_rate Sample rate in Hz
     * @param target_note Target note frequency in Hz
     * @return Processed audio samples
     */
    std::vector<float> tune_array_to_note(const std::vector<float> &samples, unsigned int sample_rate, float target_note);
} // namespace p2t

#endif // PYTOTUNE_API_H
