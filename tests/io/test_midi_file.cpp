#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <string>

#include "../test_utils.h"
#include "pytotune/io/midi_file.h"

TEST(MidiReaderTest, CanReadValidFile) {
    std::string testFile = constants::TEST_DATA_DIR + "/test.mid";

    EXPECT_NO_THROW({
        p2t::MidiFile file = p2t::MidiFile::load(testFile);
        EXPECT_NEAR(file.getLength(), 30, 0.001);
    });
}

TEST(MidiReaderTest, ReadsNotesCorrectly) {
    std::string testFile = constants::TEST_DATA_DIR + "/test.mid";

    p2t::MidiFile file = p2t::MidiFile::load(testFile);

    // Test single notes
    // First Bar, BPM = 60, Cmaj7 chord
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(0), (std::vector<int>{60}));  // C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(1), (std::vector<int>{64}));  // E4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(2), (std::vector<int>{67}));  // G4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(3), (std::vector<int>{71}));  // B4

    // Test multiple notes
    // Second Bar, BPM = 60, Cmaj7 chord over an A drone
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(4), (std::vector<int>{60, 57}));  // C4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(5), (std::vector<int>{64, 57}));  // E4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(6), (std::vector<int>{67, 57}));  // G4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(7), (std::vector<int>{71, 57}));  // B4, A3

    // Test Tempo change
    // Bar 3,4, BPM = 120, C major melody over a G drone
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(8.0), (std::vector<int>{60, 55}));   // C4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(8.5), (std::vector<int>{60, 55}));   // C4, G3 (still)
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(9.0), (std::vector<int>{64, 55}));   // E4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(9.5), (std::vector<int>{65, 55}));   // F4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(10.0), (std::vector<int>{67, 55}));  // G4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(10.5), (std::vector<int>{69, 55}));  // A4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(11.0), (std::vector<int>{71, 55}));  // B4, G3

    // Read silence
    EXPECT_TRUE(file.getActiveNotesAt(13).empty());

    // Read 2nd track
    // Bar 6, BPM = 120, C C E D
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(14.0), (std::vector<int>{60}));  // C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(14.5), (std::vector<int>{60}));  // C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(15.0), (std::vector<int>{64}));  // E4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(15.5), (std::vector<int>{62}));  // D4

    // Read 2nd track polyphonic
    // Bar 7, BPM = 60, Cmaj7 chord over a C drone
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(16), (std::vector<int>{60}));      // C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(17), (std::vector<int>{64, 60}));  // E4, C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(18), (std::vector<int>{67, 60}));  // G4, C4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(19), (std::vector<int>{71, 60}));  // B4, C4

    // Now the 2 tracks play the same melody
    // Bar 8, BPM = 60, Both Track: Cmaj7 chord, Track 2: A drone
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(20), (std::vector<int>{60, 60, 57}));  // T1: C4. T2: C4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(21), (std::vector<int>{64, 64, 57}));  // T1: E4. T2: E4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(22), (std::vector<int>{67, 67, 57}));  // T1: G4. T2: G4, A3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(23), (std::vector<int>{71, 71, 57}));  // T1: B4. T2: B4, A3.

    // Tets different melodies in the tracks
    // Bar 9, 10: BPM = 120, Track 1: Cmaj7 downwards slow, Track2: Just like Bar 3,4
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(24.0), (std::vector<int>{71, 60, 55}));  // T1: B4. T2: C4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(24.5), (std::vector<int>{71, 60, 55}));  // T1: B4. T2: C4, G3 (still)
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(25.0), (std::vector<int>{67, 64, 55}));  // T1: G4. T2: E4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(25.5), (std::vector<int>{67, 65, 55}));  // T1: G4. T2: F4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(26.0), (std::vector<int>{64, 67, 55}));  // T1: E4. T2: G4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(26.5), (std::vector<int>{64, 69, 55}));  // T1: E4. T2: A4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(27.0), (std::vector<int>{60, 71, 55}));  // T1: C4. T2: B4, G3
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(27.5), (std::vector<int>{60, 71, 55}));  // T1: C4. T2: B4, G3 (still)

    // F chord at the end, for completeness’s sake
    EXPECT_SAME_MULTISET(file.getActiveNotesAt(28), (std::vector<int>{60, 65, 69, 53}));  // T1: C4, F4, A4. T2: F3
}

TEST(MidiReaderTest, ReadOutOfRange) {
    std::string testFile = constants::TEST_DATA_DIR + "/test.mid";

    p2t::MidiFile file = p2t::MidiFile::load(testFile);

    EXPECT_TRUE(file.getActiveNotesAt(-1).empty());
    EXPECT_TRUE(file.getActiveNotesAt(file.getLength() + 1).empty());
}

TEST(MidiReaderTest, ReadWindowedNotes) {
    std::string testFile = constants::TEST_DATA_DIR + "/test.mid";

    p2t::MidiFile file = p2t::MidiFile::load(testFile);

    // One window every second.
    p2t::Windowing windowing = p2t::Windowing(100, 10);
    const float sampleRate = 10.f;

    const std::vector<std::vector<int> > &windowedNotes = file.getWindowedNotes(windowing, sampleRate).data;
    const std::vector<int> &windowedMaxNotes = file.getWindowedHighestNotes(windowing, sampleRate).data;

    EXPECT_SAME_MULTISET(windowedNotes[0], (std::vector<int>{60}));
    EXPECT_EQ(windowedMaxNotes[0], 60);

    EXPECT_SAME_MULTISET(windowedNotes[4], (std::vector<int>{60, 57}));
    EXPECT_EQ(windowedMaxNotes[4], 60);

    EXPECT_SAME_MULTISET(windowedNotes[28], (std::vector<int>{60, 65, 69, 53}));
    EXPECT_EQ(windowedMaxNotes[28], 69);
}

TEST(MidiReaderTest, ReadWindowedPitches) {
    std::string testFile = constants::TEST_DATA_DIR + "/test.mid";
    constexpr float tuning = 442;

    auto note2pitch = [](int note) {
        return static_cast<float>(tuning * std::pow(2.0, (note - 69) / 12.0));
    };

    p2t::MidiFile file = p2t::MidiFile::load(testFile);

    // One window every second.
    p2t::Windowing windowing = p2t::Windowing(100, 10);
    const float sampleRate = 10.f;

    const std::vector<std::vector<float> > &windowedPitches = file.getWindowedPitches(windowing, sampleRate, tuning).data;
    const std::vector<float> &windowedMaxPitches = file.getWindowedHighestPitches(windowing, sampleRate, tuning).data;

    EXPECT_SAME_MULTISET(windowedPitches[0], (std::vector<float>{note2pitch(60)}));
    EXPECT_FLOAT_EQ(windowedMaxPitches[0], note2pitch(60));

    EXPECT_SAME_MULTISET(windowedPitches[4], (std::vector<float>{note2pitch(60), note2pitch(57)}));
    EXPECT_FLOAT_EQ(windowedMaxPitches[4], note2pitch(60));

    EXPECT_SAME_MULTISET(windowedPitches[28], (std::vector<float>{note2pitch(60), note2pitch(65), note2pitch(69),
                                                                  note2pitch(53)}));
    EXPECT_FLOAT_EQ(windowedMaxPitches[28], note2pitch(69));
}
