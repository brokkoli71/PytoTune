#include "../include/pytotune/api.h"
#include "perfevent/PerfEvent.hpp"
#include <iostream>


void benchmark_midi() {
    std::string midiFile = "../tests/data/benchmarking/Crappy Elise.mid";
    std::string wavFile = "../tests/data/benchmarking/Crappy Elise Text.wav";
    std::string outputFile = "../tests/testoutput/Elise.wav";

    p2t::tuneToMidi(wavFile, midiFile, outputFile);
}

void benchmark_pitch_detection(int decimationFactor) {
    // std::string wavFile = "../tests/data/benchmarking/Crappy Elise Text.wav";
    std::string wavFile = "../tests/data/benchmarking/e-minor-singing-10x.wav";
    auto wav = p2t::WavFile::load(wavFile);
    auto pitchRange = p2t::VoiceRanges::HUMAN;
    int windowSize = 4096;
    int stride = 1024;
    p2t::Windowing windowing(windowSize, stride);
    p2t::YINPitchDetector ypd(windowing);
    p2t::WindowedData<float> pitches = ypd.detectPitch(wav.data(), pitchRange, 0.05, decimationFactor);
}

void benchmark_scale() {
    std::string wavFile = "../tests/data/benchmarking/e-minor-singing.wav";
    std::string outputFile = "../tests/testoutput/e-minor-singing.wav";

    p2t::Scale scale = p2t::Scale::fromName("E minor", DEFAULT_A4);
    p2t::tuneToScale(wavFile, scale, outputFile);
}
PerfEvent e;

int main(int argc, char* argv[]) {
    if (argc > 2 && std::string(argv[2]) == "false") {
        e.printHeader = false;
    }
    if (argc <= 1) {
        std::cerr << "Please specify a benchmark to run: 'midi', 'scale', or 'pitch_detection'" << std::endl;
        return 1;
    }
    std::string tag = std::string(argv[1]);
    if (tag == "midi") {
        PerfEventBlock b(e, 1000000, tag); // Counters are started in constructor
        benchmark_midi(); // Run the benchmarked code in the block
    } else if (tag == "scale") {
        PerfEventBlock b(e, 1000000, tag); // Counters are started in constructor
        benchmark_scale(); // Run the benchmarked code in the block
    } else if (tag == "pitch_detection") {
        int decimationFactor = 2;
        if (argc > 3) {
            decimationFactor = atoi(argv[3]);
        }
        PerfEventBlock b(e, 1000000, tag + std::to_string(decimationFactor));
        benchmark_pitch_detection(decimationFactor);
    } else {
        std::cerr << "Unknown benchmark: " << tag << ". Please specify 'midi', 'scale', or 'pitch_detection'" << std::endl;
        return 1;
    }
   return 0;
}

