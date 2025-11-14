//
// Created by Moritz Seppelt on 14.11.25.
//


#include <gtest/gtest.h>

#include "pytotune/data-structures/Scale.h"


// --------------------------------------------------------------
// Constructor validation
// --------------------------------------------------------------

TEST(ScaleTest, ConstructorAcceptsValidInput) {
    std::vector<float> notes = {1.0f, 1.2f, 1.5f};
    p2t::Scale s(440.0f, 2.0f, notes);

    EXPECT_FLOAT_EQ(s.getBaseNote(), 440.0f);
    EXPECT_FLOAT_EQ(s.getRepeatFactor(), 2.0f);
    EXPECT_EQ(s.getNotes(), notes);
}

TEST(ScaleTest, ConstructorRejectsNonPositiveBaseNote) {
    EXPECT_THROW(p2t::Scale(0.0f, 2.0f, {1.0f}), std::invalid_argument);
    EXPECT_THROW(p2t::Scale(-10.0f, 2.0f, {1.0f}), std::invalid_argument);
}

TEST(ScaleTest, ConstructorRejectsRepeatFactorNotGreaterThan1) {
    EXPECT_THROW(p2t::Scale(440.0f, 1.0f, {1.0f}), std::invalid_argument);
    EXPECT_THROW(p2t::Scale(440.0f, 0.5f, {1.0f}), std::invalid_argument);
}

TEST(ScaleTest, ConstructorRejectsUnsortedNotes) {
    EXPECT_THROW(p2t::Scale(440.0f, 2.0f, {1.2f, 1.1f}), std::invalid_argument);
}

TEST(ScaleTest, ConstructorRejectsNotesOutsideRange) {
    EXPECT_THROW(p2t::Scale(440.0f, 2.0f, {0.9f}), std::invalid_argument);
    EXPECT_THROW(p2t::Scale(440.0f, 2.0f, {2.1f}), std::invalid_argument);
}


// --------------------------------------------------------------
// Getter / Setter tests
// --------------------------------------------------------------

TEST(ScaleTest, SetBaseNoteAcceptsValidInput) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    s.setBaseNote(220.0f);
    EXPECT_FLOAT_EQ(s.getBaseNote(), 220.0f);
}

TEST(ScaleTest, SetBaseNoteRejectsInvalidInput) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    EXPECT_THROW(s.setBaseNote(0.0f), std::invalid_argument);
}

TEST(ScaleTest, SetRepeatFactorAcceptsValidInput) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    s.setRepeatFactor(1.5f);
    EXPECT_FLOAT_EQ(s.getRepeatFactor(), 1.5f);
}

TEST(ScaleTest, SetRepeatFactorRejectsInvalidInput) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    EXPECT_THROW(s.setRepeatFactor(1.0f), std::invalid_argument);
}

TEST(ScaleTest, SetNotesAcceptsValidInput) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    s.setNotes({1.0f, 1.3f, 1.7f});
    EXPECT_EQ(s.getNotes(), (std::vector<float>{1.0f, 1.3f, 1.7f}));
}

TEST(ScaleTest, SetNotesRejectsInvalidOrder) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    EXPECT_THROW(s.setNotes({1.2f, 1.1f}), std::invalid_argument);
}

TEST(ScaleTest, SetNotesRejectsInvalidRange) {
    p2t::Scale s(440.0f, 2.0f, {1.0f});
    EXPECT_THROW(s.setNotes({0.5f}), std::invalid_argument);
    EXPECT_THROW(s.setNotes({2.1f}), std::invalid_argument);
}


// --------------------------------------------------------------
// getClosestPitchInSclae
// --------------------------------------------------------------

TEST(ScaleTest, ClosestPitchSimple) {
    p2t::Scale s(440.0f, 2.0f, {1.0f, 1.25f, 1.5f, 1.75f});

    // This pitch is closer to 1.5 × baseNote than to 1.25 or 1.75
    float pitch = 440.0f * 1.52f;
    float expected = 440.0f * 1.5f;
    EXPECT_NEAR(s.getClosestPitchInScale(pitch), expected, 1e-5f);
}

TEST(ScaleTest, ClosestPitchInDifferentOctave) {
    p2t::Scale s(440.0f, 2.0f, {1.0f, 1.5f, 1.9f});

    // Pitch outside the base octave
    float pitch = 440.0f * 3.1f; // above 2×
    float expected = 440.0f * 3.0f; // after repeating
    EXPECT_NEAR(s.getClosestPitchInScale(pitch), expected, 1e-5f);
}

TEST(ScaleTest, ClosestPitchCanRoundToNextOcatave) {
    p2t::Scale s(440.0f, 2.0f, {1.0f, 1.25f, 1.5f, 1.75f});

    // Pitch outside the base octave
    float pitch = 440.0f * 0.24f;
    float expected = 440.0f * 0.25f; // after repeating
    EXPECT_NEAR(s.getClosestPitchInScale(pitch), expected, 1e-5f);
}

TEST(ScaleTest, ClosestPitchCanRoundToLowerOcatave) {
    // Note: This only happens when the root note is not part of the scale.
    // Judge yourself whether this is practical, but the implementation allows it
    p2t::Scale s(440.0f, 2.0f, {1.25f, 1.5f, 1.75f});

    // Pitch outside the base octave
    float pitch = 440.0f * 4.01f; // Just over 2 octaves
    float expected = 440.0f * 1.75f * 2; // Last not in octave below
    EXPECT_NEAR(s.getClosestPitchInScale(pitch), expected, 1e-5f);
}


// --------------------------------------------------------------
// fromName()
// --------------------------------------------------------------

TEST(ScaleTest, FromNameRejectsInvalidString) {
    EXPECT_THROW(p2t::Scale::fromName("not a scale"), std::invalid_argument);
    EXPECT_THROW(p2t::Scale::fromName("major"), std::invalid_argument);
}

TEST(ScaleTest, FromNameSimpleScale) {
    p2t::Scale s = p2t::Scale::fromName("A major", 440.0f);
    EXPECT_EQ(s.getNotes().size(), 7);
    EXPECT_FLOAT_EQ(s.getBaseNote(), 440.0f);
}

TEST(ScaleTest, FromNameTestCreationOfAllDocumentedScales) {
    const std::vector<std::string> roots = {"A", "B", "C", "D", "E", "F", "G"};
    const std::vector<std::string> accidentals = {"", "#", "b"};
    const std::vector<std::string> modes = {
        "major just", "minor just", "ionian", "major", "dorian", "phrygian", "lydian", "mixolydian", "aeolian", "minor",
        "locrian", "harmonic minor", "melodic minor", "pentatonic major", "pentatonic minor", "blues", "chromatic",
        "whole-tone", "quarter-tone", "edo19", "bohlen-pierce"
    };
    EXPECT_NO_THROW({
        // atonal
        p2t::Scale::fromName("chromatic");
        p2t::Scale::fromName("whole-tone");
        p2t::Scale::fromName("quarter-tone");

        // All tonal ones
        for (const auto &root: roots)
        for (const auto &acc: accidentals)
        for (const auto &mode: modes)
        p2t::Scale::fromName(root + acc + " " + mode);
        });
}

// --------------------------------------------------------------
// fromModeName()
// --------------------------------------------------------------

TEST(ScaleTest, FromModeNameRejectsUnknownMode) {
    EXPECT_THROW(p2t::Scale::fromModeName("superlocrian++", 440.0f),
                 std::invalid_argument);
}

TEST(ScaleTest, FromModeNameCreatesScale) {
    p2t::Scale s = p2t::Scale::fromModeName("major", 440.0f);
    EXPECT_GT(s.getNotes().size(), 0);
}


// --------------------------------------------------------------
// fromMode()
// --------------------------------------------------------------

TEST(ScaleTest, FromModeCreatesScale) {
    p2t::Mode mode{2.0f, {1.0f, 1.25f, 1.5f, 1.75f}};
    p2t::Scale s = p2t::Scale::fromMode(mode, 440.0f);

    EXPECT_EQ(s.getNotes(), (mode.notes));
    EXPECT_FLOAT_EQ(s.getBaseNote(), 440.0f);
}
