#ifndef PYTOTUNE_API_H
#define PYTOTUNE_API_H

#include <string>

#include "algorithms/yin_pitch_detector.h"
#include "data-structures/scale.h"

namespace p2t {
    /**
     * Tune a wav file using a midi file as reference.
     * @param wavPath Path to the input wav file
     * @param midiPath Path to the input midi file
     * @param outPath Path to the output wav file
     * @param pitchRange The maximum pitch detection range
     *
     */
    void matchMidi(const std::string &wavPath, const std::string &midiPath, const std::string &outPath,
                   PitchRange pitchRange = VoiceRanges::HUMAN);

    /**
     * Tune a wav file to a musical scale.
     * @param wavPath Path to the input wav file
     * @param scale The scale object
     * @param outPath Path to the output wav file
     * @param pitchRange The maximum pitch detection range
     */
    void roundToScale(const std::string &wavPath, const Scale &scale, const std::string &outPath,
                      PitchRange pitchRange = VoiceRanges::HUMAN);
} // namespace p2t

#endif // PYTOTUNE_API_H
