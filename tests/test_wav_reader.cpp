//
// Created by hannes on 12/11/2025.
//
#include "pytotune/wav_reader.h"
#include <gtest/gtest.h>

#include <string>

#include "test_utils.h"


TEST(WavReaderTest, DoesNotThrowOnValidFile) {
    std::string testFile = std::string(TEST_DATA_DIR) + "/pcm.wav";

    EXPECT_NO_THROW({
        p2t::WavReader reader(testFile);
        const auto& data = reader.data();

        EXPECT_GT(data.sampleRate, 0);
        EXPECT_FALSE(data.samples.empty());
        });
}

TEST(WavReaderTest, ThrowsOnInvalidFile) {
    std::string testFile = std::string(TEST_DATA_DIR) + "/invalid.wav";

    EXPECT_THROW({
                 p2t::WavReader reader(testFile);
                 }, std::runtime_error);
}
