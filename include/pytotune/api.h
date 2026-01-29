#ifndef PYTOTUNE_API_H
#define PYTOTUNE_API_H

#include <string>

namespace p2t {

    /**
     * Tune a wav file using a midi file as reference.
     * @param wav_path Path to the input wav file
     * @param midi_path Path to the input midi file
     * @param out_path Path to the output wav file
     */
    void tune_to_midi(const std::string& wav_path, const std::string& midi_path, const std::string& out_path);

    /**
     * Tune a wav file to a musical scale.
     * @param wav_path Path to the input wav file
     * @param scale_name Name of the scale (e.g. "C Major", "D Minor")
     * @param tuning Tuning frequency in Hz (e.g. 440.0)
     * @param out_path Path to the output wav file
     */
    void tune_to_scale(const std::string& wav_path, const std::string& scale_name, float tuning, const std::string& out_path);

} // namespace p2t

#endif // PYTOTUNE_API_H
