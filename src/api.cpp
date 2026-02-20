#include "pytotune/api.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"

namespace p2t {
    void tune_to_midi(const std::string &wav_path, const std::string &midi_path, const std::string &out_path) {
        auto wav = WavFile::load(wav_path);
        auto midi = MidiFile::load(midi_path);

        int windowSize = 8192;
        int stride = 2048;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.matchMidi(wav, midi, windowing, 0.0f);
        outWav.store(out_path);
    }

    void tune_to_scale(const std::string &wav_path, const Scale &scale,
                       const std::string &out_path) {
        auto wav = WavFile::load(wav_path);

        int windowSize = 8192;
        int stride = 2048;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        WavFile outWav = pipeline.roundToScale(wav, scale, windowing);
        outWav.store(out_path);
    }

    std::vector<float> tune_array_to_scale(const std::vector<float> &samples, unsigned int sample_rate, const Scale &scale) {
        int windowSize = 8192;
        int stride = 2048;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        return pipeline.processArrayToScale(samples, sample_rate, scale, windowing);
    }

    std::vector<float> tune_array_to_note(const std::vector<float> &samples, unsigned int sample_rate, float target_note) {
        int windowSize = 8192;
        int stride = 2048;
        Windowing windowing(windowSize, stride);

        PitchCorrectionPipeline pipeline;
        return pipeline.processArrayToNote(samples, sample_rate, target_note, windowing);
    }
} // namespace p2t
