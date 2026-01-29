#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "pytotune/api.h"
#include "pytotune/io/midi_file.h"
#include "test_utils.h"

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
    std::string outputFile = constants::TEST_OUTPUT_DIR + "/test_tune_to_scale.wav";
    p2t::Scale scale = p2t::Scale::fromName("C major");

    EXPECT_NO_THROW({
        p2t::tune_to_scale(wavFile, scale, outputFile);
    });
}
