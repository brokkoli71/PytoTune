#ifndef PYTOTUNE_API_H
#define PYTOTUNE_API_H

#include <string>

#include "algorithms/yin_pitch_detector.h"
#include "data-structures/scale.h"

namespace p2t {
    /**
     * Tune a wav file using a midi file as reference.
     * @param wav_path Path to the input wav file
     * @param midi_path Path to the input midi file
     * @param out_path Path to the output wav file
     * @param pitch_range The maximum pitch detection range
     *
     */
    void tune_to_midi(const std::string &wav_path, const std::string &midi_path, const std::string &out_path,
                      PitchRange pitch_range);

    /**
     * Tune a wav file to a musical scale.
     * @param wav_path Path to the input wav file
     * @param scale The scale object
     * @param out_path Path to the output wav file
     * @param pitch_range The maximum pitch detection range
     */
    void tune_to_scale(const std::string &wav_path, const Scale &scale, const std::string &out_path,
                       PitchRange pitch_range);
} // namespace p2t

#endif // PYTOTUNE_API_H
