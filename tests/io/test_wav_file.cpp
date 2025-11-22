#include "../../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "../test_utils.h"


TEST(WavFileTest, DoesNotThrowOnValidFile) {
    std::string testFile = std::string(TEST_DATA_DIR) + "pcm.wav";

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
    std::string testFileAF1 = std::string(TEST_DATA_DIR) + "sin_f440_i80_sr44100_af1.wav";
    std::string testFileAF3 = std::string(TEST_DATA_DIR) + "sin_f440_i80_sr44100_af3.wav";

    EXPECT_NO_THROW({
        p2t::WavFile readerAF1 = p2t::WavFile::load(testFileAF1);
        const auto& dataAF1 = readerAF1.data();

        EXPECT_EQ(dataAF1.sampleRate, 44100);
        EXPECT_EQ(dataAF1.numChannels, 1);

        p2t::WavFile readerAF3 = p2t::WavFile::load(testFileAF3);
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
    std::string testFile = std::string(TEST_DATA_DIR) + "invalid.wav";

    EXPECT_THROW({
                 p2t::WavFile::load(testFile);
                 }, std::runtime_error);
}

TEST(WavFileTest, RoundTrip) {
    std::string testFile = std::string(TEST_DATA_DIR) + "pcm.wav";
    std::string testOutput = std::string(TEST_OUTPUT_DIR) + "testRoundTrip.wav";

    for (int audioFormats[] = {1, 3}; const int af : audioFormats) {
        EXPECT_NO_THROW({
            p2t::WavFile reader = p2t::WavFile::load(testFile);
            const auto& data = reader.data();

            reader.store(testOutput, af);
            p2t::WavFile reader2 = p2t::WavFile::load(testOutput);
            const auto& data2 = reader2.data();
            EXPECT_EQ(data.sampleRate, data2.sampleRate);
            EXPECT_EQ(data.numChannels, data2.numChannels);
            ASSERT_EQ(data.samples.size(), data2.samples.size());
            for (size_t i = 0; i < data.samples.size(); ++i) {
                EXPECT_NEAR(data.samples[i], data2.samples[i], 1e-6);
            }
        });
        // Clean up
        std::remove(testOutput.c_str());
    }
}
