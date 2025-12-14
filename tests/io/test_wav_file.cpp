#include <cmath>

#include "../../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "../test_utils.h"


TEST(WavFileTest, DoesNotThrowOnValidFile) {
    std::string testFile = constants::TEST_DATA_DIR + "/pcm.wav";

    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(testFile);
        const auto& data = reader.data();

        EXPECT_GT(data.sampleRate, 0);
        EXPECT_FALSE(data.samples.empty());
        });
}
//

TEST(WavFileTest, AudioFormatsPCMAndFloatAreEquivalent)
{
    EXPECT_NO_THROW({
        p2t::WavFile readerAF1 = p2t::WavFile::load(constants::SIN_FILE);
        const auto& dataAF1 = readerAF1.data();

        EXPECT_EQ(dataAF1.sampleRate, 44100);
        EXPECT_EQ(dataAF1.numChannels, 1);

        p2t::WavFile readerAF3 = p2t::WavFile::load(constants::SIN_AF3_FILE);
        const auto& dataAF3 = readerAF3.data();

        EXPECT_EQ(dataAF3.sampleRate, 44100);
        EXPECT_EQ(dataAF3.numChannels, 1);

        // expect the samples to be approximately equal
        ASSERT_EQ(dataAF1.samples.size(), dataAF3.samples.size());
        for (size_t i = 0; i < dataAF1.samples.size(); ++i) {
            EXPECT_NEAR(dataAF1.samples[i], dataAF3.samples[i], 1e-3);
        }
    });

}

TEST(WavFileTest, ThrowsOnInvalidFile) {
    std::string testFile = constants::INVALID_FILE;

    EXPECT_THROW({
                 p2t::WavFile::load(testFile);
                 }, std::runtime_error);
}

TEST(WavFileTest, CorrectDataSin) {
    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(constants::SIN_FILE);
        const auto& data = reader.data();
        const float sin_freq = static_cast<float>(data.sampleRate) / 440;
        const float sin_amp = 0.8f;
        EXPECT_NEAR(data.samples[sin_freq/4], sin_amp, 1e-3);
        // for (size_t i = 0; i < sin_freq; i++){
        for (size_t i = 0; i < data.samples.size(); i++){
            float expected = sin_amp * std::sin(2 * M_PI / sin_freq * i);
            EXPECT_NEAR(data.samples[i], expected, 1e-3);
        }

        });
}

TEST(WavFileTest, StereoAsMono) {
    EXPECT_NO_THROW({
        const p2t::WavFile reader = p2t::WavFile::load(constants::PIANO_FILE);
        const auto& data = reader.data();
        EXPECT_EQ(data.numChannels, 1);
        });
}
