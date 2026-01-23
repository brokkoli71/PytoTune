#include <gtest/gtest.h>

#include "../test_utils.h"
#include "pytotune/algorithms/pitch_correction_pipeline.h"

/*
TEST(TestPitchPitchCorrectionPipeline, Playground) {
    // std::string testFile = constants::TEST_DATA_DIR + "/sin-raise_fstart200_fend1000_i80_sr44100_af1.wav";
    std::string testFile = constants::TEST_DATA_DIR + "/alle_meine_entchen.wav";

    p2t::WavFile src = p2t::WavFile::load(testFile);
    p2t::PitchCorrectionPipeline pcp{};
    p2t::Scale scale = p2t::Scale::fromName("G major");
    p2t::WavFile out = pcp.roundToScale(src, scale);


    out.store(constants::TEST_OUTPUT_DIR + "/test.wav");
}


TEST(TestPitchPitchCorrectionPipeline, Playground2) {
    std::string testFile = constants::TEST_DATA_DIR + "/poem.wav";
    std::string midi = constants::TEST_DATA_DIR + "/test.mid";

    p2t::WavFile src = p2t::WavFile::load(testFile);
    p2t::MidiFile midiFile = p2t::MidiFile::load(midi);
    p2t::PitchCorrectionPipeline pcp{};
    p2t::Scale scale = p2t::Scale::fromName("G major");
    p2t::WavFile out = pcp.matchMidi(src, midiFile);


    out.store(constants::TEST_OUTPUT_DIR + "/test.wav");
}
*/
