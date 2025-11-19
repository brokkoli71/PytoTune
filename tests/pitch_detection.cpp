#include "../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "test_utils.h"
#include "pytotune/yin_pitch_detector.h"

TEST(PitchDetectionTest, DoesNotThrow)
{
    std::string testFile = std::string(TEST_DATA_DIR) + "/pcm.wav";

    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(testFile);
        const auto& data = reader.data();

        auto detection = p2t::YINPitchDetector(256, 0).detect_pitch(&data, 20, 200, 0.1f );

        for (const auto& pitch : detection.pitch_values)
        {
            std::cout << pitch << std::endl;
        }


        });
}