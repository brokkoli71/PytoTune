#include "../../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "../test_utils.h"
#include "../../include/pytotune/algorithms/yin_pitch_detector.h"

TEST(PitchDetectionTest, DetectSineWavePitch)
{
    std::string testFile = std::string(TEST_DATA_DIR) + "sin_f440_i80_sr44100_af1.wav";

    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(testFile);
        const auto& data = reader.data();

        auto detection = p2t::YINPitchDetector(2024, 0).detect_pitch(&data, 20, 2000, 0.2f );

        for (const auto& pitch : detection.pitch_values)
        {
            // Expect the detected pitch to be approximately 440 Hz
            EXPECT_NEAR(pitch, 440.0f, 0.1f);
        }


        });
}