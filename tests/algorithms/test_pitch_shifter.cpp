#include "../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include "../test_utils.h"
#include "pytotune/yin_pitch_detector.h"
#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/io/midi_file.h"

TEST(PitchShifterTest, Playground) {
    std::string wavFile = std::string(TEST_DATA_DIR) + "/sin_f440_i80_sr44100_af1.wav";
    p2t::WavFile reader = p2t::WavFile::load(wavFile);

    const auto &data = reader.data();
    auto detection = p2t::YINPitchDetector(2024, 0).detect_pitch(&data, 20, 2000, 0.2f);

    std::string midiFile_ = std::string(TEST_DATA_DIR) + "/test.mid";
    p2t::MidiFile midiFile = p2t::MidiFile::load(midiFile_);

    p2t::PitchShifter ps = p2t::PitchShifter::createConstantPitchMatcher(detection, 440);

    p2t::WavData outWavData;
    outWavData.sampleRate = data.sampleRate;
    outWavData.numChannels = data.numChannels;
    outWavData.samples = std::vector<float>(data.samples.size(), 0.0f);
    ps.process(&data.samples[0], &outWavData.samples[0], data.samples.size());

    for (auto sample: outWavData.samples) {
        std::cout << sample << ", ";
    }
}
