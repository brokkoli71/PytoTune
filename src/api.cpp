#include "pytotune/api.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"

namespace p2t {
    void tune_to_midi(const std::string &wav_path, const std::string &midi_path, const std::string &out_path,
                      const PitchRange pitch_range) {
        auto wav = WavFile::load(wav_path);
        auto midi = MidiFile::load(midi_path);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.matchMidi(wav, midi, windowing, DEFAULT_A4, pitch_range);
        outWav.store(out_path);
    }

    void tune_to_scale(const std::string &wav_path, const Scale &scale,
                       const std::string &out_path, const PitchRange pitch_range) {
        auto wav = WavFile::load(wav_path);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.roundToScale(wav, scale, windowing, pitch_range);
        outWav.store(out_path);
    }
} // namespace p2t
