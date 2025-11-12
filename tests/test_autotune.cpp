//
// Created by hannes on 12/11/2025.
//
#include "pytotune/wav_reader.h"
#include <gtest/gtest.h>

#include <string>

auto TEST_DATA_DIR = "../tests/data";

TEST(WavReaderTest, DoesNotThrowOnValidFile) {
  std::string testFile = std::string(TEST_DATA_DIR) + "/pcm.wav";

  EXPECT_NO_THROW({
      WavReader reader(testFile);
      const auto& data = reader.data();

      EXPECT_GT(data.sampleRate, 0);
      EXPECT_FALSE(data.samples.empty());
  });
}

TEST(WavReaderTest, ThrowsOnInvalidFile) {
  std::string testFile = std::string(TEST_DATA_DIR) + "/invalid.wav";

  EXPECT_THROW({
      WavReader reader(testFile);
  }, std::runtime_error);
}
