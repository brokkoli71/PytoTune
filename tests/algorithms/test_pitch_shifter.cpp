#include <gtest/gtest.h>

#include <cmath>

#include "../reference/pitch_shifter_reference.h"
#include "../test_utils.h"
#include "pytotune/algorithms/PitchShifter.h"
#include "pytotune/io/wav_file.h"

TEST(TestPitchShifterReference, Playground) {
    std::string testFile = std::string(TEST_DATA_DIR) + "voice_f440_sr44100.wav";

    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();
    std::vector<float> out(data.samples.size(), 0.0f);

    smbPitchShift(1.05946309436f, data.samples.size(), 4096, 4, data.sampleRate, (float *) &data.samples[0], &out[0]);

    p2t::WavFile newFile({data.sampleRate, 2, out});
    newFile.store(std::string(TEST_OUTPUT_DIR) + "test.wav");
}

TEST(TestPitchShifter, Playground) {
    std::string testFile = std::string(TEST_DATA_DIR) + "voice-majorscale_fstart220_fend440_cd6_tail_pause.wav";

    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();
    p2t::PitchShifter ps({4096, 4096 / 4}, data.sampleRate);
    auto out = ps.run(data.samples, 0.9f);

    p2t::WavFile newFile({data.sampleRate, 2, out});
    newFile.store(std::string(TEST_OUTPUT_DIR) + "test.wav");
}

TEST(TestPitchShifter, ResultEqualsReferenceCode) {
    std::string testFile = std::string(TEST_DATA_DIR) + "voice-majorscale_fstart220_fend440_cd6_tail_pause.wav";
    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();

    p2t::PitchShifter ps({4096, 4096 / 4}, data.sampleRate);
    auto out1 = ps.run(data.samples, 0.9f);

    std::vector<float> out2(data.samples.size(), 0.0f);
    smbPitchShift(0.9f, data.samples.size(), 4096, 4, data.sampleRate, (float *) &data.samples[0], &out2[0]);

    EXPECT_NEAR_VEC_EPS(out1, out2, 1e-2f);
}
