#include "perfevent/PerfEvent.hpp"
#include <iostream>

#include "pytotune/algorithms/pitch_correction_pipeline.h"

constexpr auto pitchRange = p2t::VoiceRanges::HUMAN;
constexpr int windowSize = 4096;
constexpr int stride = 1024;

const std::string wavPath = "../tests/data/benchmarking/e-minor-singing-10x.wav";

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

int main(int argc, char* argv[]) {
    if (argc > 2 && std::string(argv[2]) == "false") {
        e.printHeader = false;
    }
    if (argc <= 1) {
        std::cerr << "Please specify a benchmark to run: 'detection' or 'correction'" << std::endl;
        return 1;
    }

    std::string tag = std::string(argv[1]);
    p2t::Windowing windowing(windowSize, stride);

    if (tag == "detection") {
        auto wav = p2t::WavFile::load(wavPath);
        p2t::PitchCorrectionPipeline pipeline;

        for (int decimation : {1, 2, 4, 8}) {
            const std::string ompTag = (std::getenv("OMP_NUM_THREADS") && std::string(std::getenv("OMP_NUM_THREADS")) == "1")
                                       ? "_omp=off" : "_omp=on";
            PerfEventBlock b(e, 1000000, "d" + std::to_string(decimation) + std::string(HWY_TAG) + ompTag);
            if (decimation>1) e.printHeader = false;
            pipeline.detectPitch(wav, windowing, pitchRange, 0.05f, decimation);
        }

    } else if (tag == "correction") {
        auto wav = p2t::WavFile::load(wavPath);
        const auto scale = p2t::Scale::fromName("E minor");
        p2t::PitchCorrectionPipeline pipeline;

        // Run detection outside the benchmarked block
        const auto pitches = pipeline.detectPitch(wav, windowing, pitchRange);
        const auto factors = scale.getPitchCorrectionFactors(pitches.data);

        PerfEventBlock b(e, 1000000, std::string(TWIDDLES_TAG) + std::string(WINDOWING_TAG));
        pipeline.shiftPitch(wav, windowing, factors);

    } else {
        std::cerr << "Unknown benchmark: " << tag << ". Please specify 'detection' or 'correction'" << std::endl;
        return 1;
    }
    return 0;
}

