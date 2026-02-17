#ifndef PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#define PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"
#include  "pytotune/data-structures/windowing.h"


namespace p2t {
#define DEFAULT_WINDOWING Windowing(4096, 4096 / 4)

    class PitchCorrectionPipeline {
    public:
        WavFile matchMidi(const WavFile &src,
                          const MidiFile &midiFile,
                          Windowing windowing = DEFAULT_WINDOWING,
                          float tuning = DEFAULT_A4);

        WavFile roundToScale(const WavFile &src,
                             const Scale &scale,
                             Windowing windowing = DEFAULT_WINDOWING);

        std::vector<float> processArrayToScale(const std::vector<float> &samples,
                                                unsigned int sample_rate,
                                                const Scale &scale,
                                                Windowing windowing = DEFAULT_WINDOWING);

        std::vector<float> processArrayToNote(const std::vector<float> &samples,
                                               unsigned int sample_rate,
                                               float target_note,
                                               Windowing windowing = DEFAULT_WINDOWING);
    };
}

#endif //PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
