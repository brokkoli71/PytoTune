#ifndef PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#define PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#include "yin_pitch_detector.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"
#include  "pytotune/data-structures/windowing.h"


namespace p2t {
#define DEFAULT_WINDOWING Windowing(4096, 4096 / 4)

    class PitchCorrectionPipeline {
    public:
        /// detect pitch per window. Exposed for benchmarking.
        WindowedData<float> detectPitch(const WavFile &src,
                                        Windowing windowing,
                                        PitchRange pitchRange,
                                        float threshold = DEFAULT_THRESHOLD,
                                        int decimationFactor = DEFAULT_DECIMATION_FACTOR) const;

        /// apply correction factors and return the shifted audio. Exposed for benchmarking.
        WavFile shiftPitch(const WavFile &src,
                           Windowing windowing,
                           const std::vector<float> &correctionFactors) const;

        WavFile matchMidi(const WavFile &src,
                          const MidiFile &midiFile,
                          Windowing windowing = DEFAULT_WINDOWING,
                          float tuning = DEFAULT_A4, PitchRange pitchRange = VoiceRanges::HUMAN) const;

        WavFile roundToScale(const WavFile &src,
                             const Scale &scale,
                             Windowing windowing = DEFAULT_WINDOWING, PitchRange pitchRange = VoiceRanges::HUMAN) const;
    };
}

#endif //PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
