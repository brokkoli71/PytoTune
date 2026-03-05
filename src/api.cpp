#include "pytotune/api.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"

namespace p2t {
    void tuneToMidi(const std::string &wavPath, const std::string &midiPath, const std::string &outPath,
                      const PitchRange pitchRange) {
        auto wav = WavFile::load(wavPath);
        auto midi = MidiFile::load(midiPath);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.matchMidi(wav, midi, windowing, DEFAULT_A4, pitchRange);
        outWav.store(outPath);
    }

    void tuneToScale(const std::string &wavPath, const Scale &scale,
                       const std::string &outPath, const PitchRange pitchRange) {
        auto wav = WavFile::load(wavPath);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.roundToScale(wav, scale, windowing, pitchRange);
        outWav.store(outPath);
    }
} // namespace p2t
