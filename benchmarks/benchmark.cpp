#include "perfevent/PerfEvent.hpp"
#include <iostream>

#include "pytotune/algorithms/pitch_correction_pipeline.h"
#include "pytotune/io/midi_file.h"

constexpr auto pitchRange = p2t::VoiceRanges::HUMAN;
constexpr int windowSize = 4096;
constexpr int stride = 1024;

const std::string wavPathCorrection = "../tests/data/benchmarking/Crappy Elise Text.wav";
const std::string midiPath = "../tests/data/benchmarking/Crappy Elise.mid";
const std::string wavPathDetection = "../tests/data/benchmarking/e-minor-singing-10x.wav";

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
        auto wav = p2t::WavFile::load(wavPathDetection);
        p2t::YINPitchDetector ypd(windowing);

        for (int decimation : {1, 2, 4, 8}) {
            PerfEventBlock b(e, 1000000, tag + "_d" + std::to_string(decimation));
            ypd.detectPitch(wav.data(), pitchRange, 0.05f, decimation);
            e.printHeader = false;
        }

    } else if (tag == "correction") {
        auto wav = p2t::WavFile::load(wavPathCorrection);
        auto midi = p2t::MidiFile::load(midiPath);

        // Run detection outside the benchmarked block
        p2t::YINPitchDetector ypd(windowing);
        p2t::WindowedData<float> pitches = ypd.detectPitch(wav.data(), pitchRange);

        p2t::PitchCorrectionPipeline pipeline;
        PerfEventBlock b(e, 1000000, tag);
        pipeline.valuesPitchedToMidi(wav, midi, pitches, windowing);

    } else {
        std::cerr << "Unknown benchmark: " << tag << ". Please specify 'detection' or 'correction'" << std::endl;
        return 1;
    }
    return 0;
}

