#include "pytotune/api.h"
#include "pytotune/io/wav_file.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"

namespace p2t {
    void tune_to_midi(const std::string& wav_path, const std::string& midi_path, const std::string& out_path) {
        auto wav = WavFile::load(wav_path);
        auto midi = MidiFile::load(midi_path);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.matchMidi(wav, midi, windowing, 0.0f);
        outWav.store(out_path);
    }

    void tune_to_scale(const std::string& wav_path, const std::string& scale_name, float tuning, const std::string& out_path) {
        auto wav = WavFile::load(wav_path);
        auto scale = Scale::fromName(scale_name, tuning);

        int windowSize = 4096;
        int stride = 1024;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.roundToScale(wav, scale, windowing);
        outWav.store(out_path);
    }
} // namespace p2t