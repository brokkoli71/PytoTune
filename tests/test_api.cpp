#include <gtest/gtest.h>

#include <string>
#include <algorithm>
#include <cmath>

#include "test_utils.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/api.h"

TEST(PythonBindingsTest, TuneToMidi) {
    std::string midiFile = constants::TEST_DATA_DIR + "/test.mid";
    std::string wavFile = constants::VOICE_F400_SR4100;
    std::string outputFile = constants::TEST_OUTPUT_DIR + "/test_tune_to_midi.wav";

    EXPECT_NO_THROW({
        p2t::tune_to_midi(wavFile, midiFile, outputFile);
        });
}

TEST(PythonBindingsTest, TuneToScale) {
    std::string wavFile = constants::VOICE_F400_SR4100;
    std::string scale = "C major";
    std::string outputFile = constants::TEST_OUTPUT_DIR + "/test_tune_to_scale.wav";
    float tuning = 440.0f;

    EXPECT_NO_THROW({
        p2t::tune_to_scale(wavFile, scale, tuning, outputFile);
        });
}