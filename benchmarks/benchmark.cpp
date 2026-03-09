#include <iostream>

#include "perfevent/PerfEvent.hpp"
#include "pytotune/algorithms/pitch_correction_pipeline.h"

constexpr auto pitchRange = p2t::VoiceRanges::HUMAN;
constexpr int windowSize = 4096;
constexpr int stride = 1024;

const std::string wavPathScale = "../tests/data/benchmarking/e-minor-singing-10x.wav";
const std::string wavPathMidi = "../tests/data/benchmarking/for-elise-text-10x.wav";
const std::string midiPath = "../tests/data/benchmarking/for-elise-10x.mid";

#if USE_HWY
constexpr std::string_view HWY_TAG = "_hwy=on";
#else
constexpr std::string_view HWY_TAG = "_hwy=off";
#endif

#if USE_PREDEFINED_TWIDDLES
constexpr std::string_view TWIDDLES_TAG = "_tw=on";
#else
constexpr std::string_view TWIDDLES_TAG = "_tw=off";
#endif

#ifdef REIMPLEMENTED_WINDOWING
constexpr std::string_view WINDOWING_TAG = "_win=on";
#else
constexpr std::string_view WINDOWING_TAG = "_win=off";
#endif

PerfEvent e;

int main(int argc, char *argv[]) {
    std::string mode = "midi"; // or "scale"
    if (argc > 2) {
        if (std::string(argv[2]) == "true")
            e.printHeader = true;
        else if (std::string(argv[2]) == "false")
            e.printHeader = false;
        else if (std::string(argv[2]) == "midi")
            mode = "midi";
        else if (std::string(argv[2]) == "scale")
            mode = "scale";
        else {
            std::cerr << "Unknown second argument: " << argv[2] << ". Expected 'true', 'false', 'midi' or 'scale'" << std::endl;
            return 1;
        }
    }
    if (argc > 3 && (std::string(argv[3]) == "midi" || std::string(argv[3]) == "scale")) {
        mode = std::string(argv[3]);
    }

    if (argc <= 1) {
        std::cerr << "Please specify a benchmark to run: 'detection', 'correction', or 'pipeline'" << std::endl;
        return 1;
    }

    std::string tag = std::string(argv[1]);
    p2t::Windowing windowing(windowSize, stride);

    if (tag == "detection") {
        p2t::WavFile wav = p2t::WavFile::load(wavPathMidi);
        if (mode == "scale")
            wav = p2t::WavFile::load(wavPathScale);

        p2t::PitchCorrectionPipeline pipeline;

        for (int decimation : {1, 2, 4, 8}) {
            const std::string ompTag = (std::getenv("OMP_NUM_THREADS") && std::string(std::getenv("OMP_NUM_THREADS")) ==
                                                                              "1")
                                           ? "_omp=off"
                                           : "_omp=on";
            PerfEventBlock b(e, 1000000, "d" + std::to_string(decimation) + std::string(HWY_TAG) + ompTag);
            if (decimation > 1) e.printHeader = false;
            pipeline.detectPitch(wav, windowing, pitchRange, 0.05f, decimation);
        }
        return 0;
    }
    if (tag == "correction") {
        if (mode == "midi") {
            auto wav = p2t::WavFile::load(wavPathMidi);
            auto midi = p2t::MidiFile::load(midiPath);
            p2t::PitchCorrectionPipeline pipeline;

            // Run detection outside the benchmarked block
            const auto pitches = pipeline.detectPitch(wav, windowing, pitchRange);
            const auto factors = midi.getPitchCorrectionFactors(pitches, windowing, wav.data().sampleRate);

            PerfEventBlock b(e, 1000000, mode + std::string(TWIDDLES_TAG) + std::string(WINDOWING_TAG));
            pipeline.shiftPitch(wav, windowing, factors);
            return 0;
        }
        if (mode == "scale") {
            auto wav = p2t::WavFile::load(wavPathScale);
            const auto scale = p2t::Scale::fromName("E minor");
            p2t::PitchCorrectionPipeline pipeline;

            // Run detection outside the benchmarked block
            const auto pitches = pipeline.detectPitch(wav, windowing, pitchRange);
            const auto factors = scale.getPitchCorrectionFactors(pitches.data);

            PerfEventBlock b(e, 1000000, mode + std::string(TWIDDLES_TAG) + std::string(WINDOWING_TAG));
            pipeline.shiftPitch(wav, windowing, factors);
            return 0;
        }
    }
    if (tag == "pipeline_ranges" || tag == "pipeline_windows") {
        constexpr float rangeBase = 82.0f; // ~E2
        const int rangeWidths[] = { 100, 500, 1000, 2000 };
        const int windowSizes[] = { 128, 256, 512, 1024, 2048, 4096, 8192 };

        const p2t::WavFile wav = (mode == "scale")
            ? p2t::WavFile::load(wavPathScale)
            : p2t::WavFile::load(wavPathMidi);

        p2t::MidiFile midi = p2t::MidiFile::load(midiPath);

        const p2t::Scale scale = p2t::Scale::fromName("E minor");

        p2t::PitchCorrectionPipeline pipeline;

        const std::string compileTags = std::string(HWY_TAG) + std::string(TWIDDLES_TAG) + std::string(WINDOWING_TAG);

        auto runOnce = [&](int width, int ws, const std::string &varyingTag) {
            const p2t::PitchRange range(rangeBase, rangeBase + static_cast<float>(width));
            const p2t::Windowing win(ws, ws / 4);

            for (int decimation : {1, DEFAULT_DECIMATION_FACTOR}) {
                const std::string baseTag = mode + varyingTag + "_d=" + std::to_string(decimation) + compileTags;

                p2t::WindowedData<float> pitches(win);
                {
                    PerfEventBlock b(e, 1000000, "det_" + baseTag);
                    pitches = pipeline.detectPitch(wav, win, range, DEFAULT_THRESHOLD, decimation);
                }
                e.printHeader = false;

                const std::vector<float> factors = (mode == "midi")
                    ? midi.getPitchCorrectionFactors(pitches, win, wav.data().sampleRate)
                    : scale.getPitchCorrectionFactors(pitches.data);

                {
                    PerfEventBlock b(e, 1000000, "cor_" + baseTag);
                    pipeline.shiftPitch(wav, win, factors);
                }
            }
        };

        if (tag == "pipeline_ranges") {
            for (int width : rangeWidths)
                runOnce(width, windowSize, "_r=" + std::to_string(width));
        } else {
            for (int ws : windowSizes)
                runOnce(static_cast<int>(pitchRange.max - pitchRange.min), ws, "_w=" + std::to_string(ws));
        }
        return 0;
    }
    std::cerr << "Unknown benchmark: " << tag << ". Please specify 'detection', 'correction', 'pipeline_ranges', or 'pipeline_windows' and mode must be 'midi' or 'scale'" << std::endl;
    return 1;
}
