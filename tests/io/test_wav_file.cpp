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

TEST(WavFileTest, ThrowsOnInvalidFile) {
    std::string testFile = std::string(TEST_DATA_DIR) + "/invalid.wav";

    EXPECT_THROW({
                 p2t::WavFile::load(testFile);
                 }, std::runtime_error);
}
