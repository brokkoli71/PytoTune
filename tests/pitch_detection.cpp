#include <cmath>

#include "../include/pytotune/io/wav_file.h"
#include <gtest/gtest.h>

#include <string>

#include "test_utils.h"
#include "pytotune/yin_pitch_detector.h"

TEST(PitchDetectionTest, DetectSineWavePitch) {
    EXPECT_NO_THROW({
        const p2t::WavFile reader = p2t::WavFile::load(constants::SIN_FILE);
        const auto& data = reader.data();
        const int window_size = 2024;
        const int expected_windows = std::ceil(static_cast<float>(constants::SIN_FILE_NUM_SAMPLES)/
            window_size);
        const int middle_C_freq = 261; // Frequency of Middle C (C4)

        const auto detection = p2t::YINPitchDetector(window_size, 0).detect_pitch(&data, middle_C_freq, middle_C_freq*4, 0.2f );

        EXPECT_EQ(detection.pitch_values.size() , expected_windows);

        for (const auto& pitch : detection.pitch_values)
        {
        // Expect the detected pitch to be approximately 440 Hz
        EXPECT_NEAR(pitch, 440.0f, 0.1f);
        }
        });
}

TEST(PitchDetectionTest, DetectSineWavePitchWithOverlap)
{
    EXPECT_NO_THROW({
        const p2t::WavFile reader = p2t::WavFile::load(constants::SIN_FILE);
        const auto& data = reader.data();
        const int window_size = 2024;
        const int window_overlap = 512;
        const int expected_windows = std::ceil(static_cast<float>(constants::SIN_FILE_NUM_SAMPLES)/
            (window_size-window_overlap));
        const int middle_C_freq = 261; // Frequency of Middle C (C4)

        const auto detection = p2t::YINPitchDetector(window_size, window_overlap).detect_pitch(&data, middle_C_freq, middle_C_freq*4, 0.2f );

        EXPECT_EQ(detection.pitch_values.size() , expected_windows);

        for (const auto& pitch : detection.pitch_values)
        {
        // Expect the detected pitch to be approximately 440 Hz
        EXPECT_NEAR(pitch, 440.0f, 0.1f);
        }
        });
}
TEST(PitchDetectionTest, DetectPianoPitch)
{
    std::string testFile = constants::PIANO_FILE;

    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(testFile);
        const auto& data = reader.data();

        const int middle_C_freq = 261; // Frequency of Middle C (C4)

        auto detection = p2t::YINPitchDetector(2024, 0).detect_pitch(&data, middle_C_freq/2, middle_C_freq, 0.f);
        EXPECT_GT(detection.pitch_values.size() , 0);

        for (const auto& pitch : detection.pitch_values)
        {
        std::cout << "Detected pitch: " << pitch << " Hz" << std::endl;
            // Expect the detected pitch to be approximately 220 Hz
            EXPECT_NEAR(pitch, 220.0f, 1.0f);
        }
        });
}

TEST(PitchDetectionTest, DetectStringsPitch)
{
    std::string testFile = constants::STRINGS_FILE;

    EXPECT_NO_THROW({
        p2t::WavFile reader = p2t::WavFile::load(testFile);
        const auto& data = reader.data();

        const int middle_C_freq = 261; // Frequency of Middle C (C4)

        auto detection = p2t::YINPitchDetector(12312, 0).detect_pitch(&data, middle_C_freq, middle_C_freq*2, 0.f);
        EXPECT_GT(detection.pitch_values.size() , 0);

        for (const auto& pitch : detection.pitch_values)
        {
        std::cout << "Detected pitch: " << pitch << " Hz" << std::endl;
            // Expect the detected pitch to be approximately 220 Hz
            EXPECT_NEAR(pitch, 440.0f, 1.0f);
        }
        });
}
