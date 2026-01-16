//
// Created by Moritz Seppelt on 12.12.25.
//

#ifndef PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#define PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"
#include  "pytotune/data-structures/windowing.h"


namespace p2t {
#define DEFAULT_WINDOWING Windowing(4096, 4096 / 4, 0)

    class PitchCorrectionPipeline {
    public:
        WavFile matchMidi(const WavFile &src,
                          const MidiFile &midiFile,
                          Windowing windowing = DEFAULT_WINDOWING,
                          float tuning = DEFAULT_A4);

        WavFile roundToScale(const WavFile &src,
                             const Scale &scale,
                             Windowing windowing = DEFAULT_WINDOWING,
                             float tuning = DEFAULT_A4);
    };
}

#endif //PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
