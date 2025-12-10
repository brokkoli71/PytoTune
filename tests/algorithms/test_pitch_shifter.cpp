#include <gtest/gtest.h>

#include <cmath>

#include "../reference/pitch_shifter_reference.h"
#include "../test_utils.h"
#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"

/* TEST(TestPitchShifterReference, Playground) {
    std::string testFile = std::string(TEST_DATA_DIR) + "voice_f440_sr44100.wav";

    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();
    std::vector<float> out(data.samples.size(), 0.0f);

    smbPitchShift(1.05946309436f, data.samples.size(), 4096, 4, data.sampleRate, (float *) &data.samples[0], &out[0]);

    p2t::WavFile newFile({data.sampleRate, 2, out});
    newFile.store(std::string(TEST_OUTPUT_DIR) + "test.wav");
}

TEST(TestPitchShifter, Playground) {
    std::string testFile = std::string(TEST_DATA_DIR) + "happy-birthday.wav";

    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();
    const p2t::Windowing w = {4096, 4096 / 4, static_cast<float>(data.sampleRate)};
    p2t::PitchShifter ps(w);
    p2t::WindowedData<float> pitchFactors = p2t::WindowedData<float>::fromLambda(
        w, data.samples.size() / w.stride, [](float t) {
            return std::sin(t * 5.0f) * 0.09 + 1.1f;
        });

    // std::string midiFile = std::string(TEST_DATA_DIR) + "test.mid";
    // p2t::MidiFile file = p2t::MidiFile::load(midiFile);
    //
    // p2t::WindowedData<float> pitchFactors = file.getWindowedHighestPitches(w);
    // std::ranges::transform(pitchFactors.data, pitchFactors.data.begin(),
    //                        [](float f) { return f / 440.0f; });

    auto out = ps.run(data.samples, pitchFactors);

    p2t::WavFile newFile({data.sampleRate, 1, out});
    newFile.store(std::string(TEST_OUTPUT_DIR) + "test.wav");
}*/

TEST(TestPitchShifter, ResultEqualsReferenceCode) {
    std::string testFile = constants::TEST_DATA_DIR +
                           "/voice-majorscale_fstart220_fend440_cd6_tail_pause.wav";
    p2t::WavFile readerAF1 = p2t::WavFile::load(testFile);
    auto &data = readerAF1.data();

    p2t::PitchShifter ps({4096, 4096 / 4, static_cast<float>(data.sampleRate)});
    auto out1 = ps.run(data.samples, 0.9f);

    std::vector<float> out2(data.samples.size(), 0.0f);
    smbPitchShift(0.9f, data.samples.size(), 4096, 4, data.sampleRate, (float *) &data.samples[0], &out2[0]);

    EXPECT_NEAR_VEC_EPS(out1, out2, 1e-2f);
    p2t::WavFile newFile1({data.sampleRate, 2, out1});
    p2t::WavFile newFile2({data.sampleRate, 2, out2});
    // newFile1.store(std::string(TEST_OUTPUT_DIR) + "test1.wav");
    // newFile2.store(std::string(TEST_OUTPUT_DIR) + "test2.wav");
}
