#include "../include/pytotune/api.h"
#include "perfevent/PerfEvent.hpp"

int main() {
    PerfEvent e;
    e.startCounters();
    // --- Benchmark midi ---
    std::string midiFile = "../tests/data/benchmarking/Crappy Elise.mid";
    std::string wavFile = "../tests/data/benchmarking/Crappy Elise Text.wav";
    std::string outputFile = "../tests/testoutput/Elise.wav";

    p2t::tuneToMidi(wavFile, midiFile, outputFile);

    e.stopCounters();
    e.printReport(std::cout, 10000000); // use n as scale factor
    return 0;
}