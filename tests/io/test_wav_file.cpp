#include "../../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "../test_utils.h"


TEST(WavFileTest, DoesNotThrowOnValidFile) {
    std::string testFile = std::string(TEST_DATA_DIR) + "/pcm.wav";

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
    std::string testFileAF1 = std::string(TEST_DATA_DIR) + "/sin_f440_i80_sr44100_af1.wav";
    std::string testFileAF3 = std::string(TEST_DATA_DIR) + "/sin_f440_i80_sr44100_af3.wav";

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
    std::string testFile = std::string(TEST_DATA_DIR) + "/invalid.wav";

    EXPECT_THROW({
                 p2t::WavFile::load(testFile);
                 }, std::runtime_error);
}
