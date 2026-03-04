//
// Created by Moritz Seppelt on 12.11.25.
//

#include "pytotune/data-structures/mode.h"
#include <cmath>
#include <algorithm>

namespace p2t {
    // ----------- EqualTemperamentUtils implementation ------------
    float EqualTemperamentUtils::semitoneRatio(const int semitone, const float division, const float repeatFactor) {
        return std::min(std::max(std::pow(repeatFactor, static_cast<float>(semitone) / division), 1.0f), repeatFactor);
    }

    std::vector<float> EqualTemperamentUtils::semitoneRatios(const std::vector<int> &semitones, float division,
                                                             float repeatFactor) {
        std::vector<float> ratios(semitones.size());
        std::transform(semitones.begin(), semitones.end(),
                       ratios.begin(),
                       [division, repeatFactor](const int s) { return semitoneRatio(s, division, repeatFactor); });
        return ratios;
    }

    // ---------------------- Just intonation ----------------------
    static const std::vector<float> JUST_MAJOR = {1.0f, 9.0f / 8, 5.0f / 4, 4.0f / 3, 3.0f / 2, 5.0f / 3, 15.0f / 8};
    static const std::vector<float> JUST_MINOR = {1.0f, 9.0f / 8, 6.0f / 5, 4.0f / 3, 3.0f / 2, 8.0f / 5, 9.0f / 5};

    // ---------------------- Equal temperament diatonic modes ----------------------
    static const std::vector<int> IONIAN_STEPS = {0, 2, 4, 5, 7, 9, 11}; // Major
    static const std::vector<int> DORIAN_STEPS = {0, 2, 3, 5, 7, 9, 10};
    static const std::vector<int> PHRYGIAN_STEPS = {0, 1, 3, 5, 7, 8, 10};
    static const std::vector<int> LYDIAN_STEPS = {0, 2, 4, 6, 7, 9, 11};
    static const std::vector<int> MIXOLYDIAN_STEPS = {0, 2, 4, 5, 7, 9, 10};
    static const std::vector<int> AEOLIAN_STEPS = {0, 2, 3, 5, 7, 8, 10}; // Minor
    static const std::vector<int> LOCRIAN_STEPS = {0, 1, 3, 5, 6, 8, 10};

    // More minor variants
    static const std::vector<int> HARMONIC_MINOR_STEPS = {0, 2, 3, 5, 7, 8, 11};
    static const std::vector<int> MELODIC_MINOR_STEPS = {0, 2, 3, 5, 7, 9, 11};

    // Chromatic scale (12-TET)
    static const std::vector<int> CHROMATIC_STEPS = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    // Whole tone scale (6-TET)
    static const std::vector<int> WHOLE_TONE_STEPS = {0, 2, 4, 6, 8, 10};

    // Quarter tone scale (24-TET)
    static const std::vector<int> QUARTER_TONE_STEPS = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
    };

    // Pentatonics
    static const std::vector<int> MAJOR_PENTATONIC_STEPS = {0, 2, 4, 7, 9};
    static const std::vector<int> MINOR_PENTATONIC_STEPS = {0, 3, 5, 7, 10};

    // Blues scale
    static const std::vector<int> BLUES_STEPS = {0, 3, 5, 6, 7, 10};

    // ---------------------- Microtonal / non-octave ----------------------
    // Example: 19-EDO
    static const std::vector<int> EDO_19_STEPS = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};

    // Bohlen-Pierce / tritave scale
    static const std::vector<int> BOHLEN_PIERCE_STEPS = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36};

    // ---------------------- Modes map ----------------------
    const std::unordered_map<std::string, Mode> MODES = {
        // Just intonation
        {"major just", {2.0f, JUST_MAJOR}},
        {"minor just", {2.0f, JUST_MINOR}},

        // Equal temperament diatonic
        {"ionian", {2.0f, EqualTemperamentUtils::semitoneRatios(IONIAN_STEPS)}},
        {"major", {2.0f, EqualTemperamentUtils::semitoneRatios(IONIAN_STEPS)}},
        {"dorian", {2.0f, EqualTemperamentUtils::semitoneRatios(DORIAN_STEPS)}},
        {"phrygian", {2.0f, EqualTemperamentUtils::semitoneRatios(PHRYGIAN_STEPS)}},
        {"lydian", {2.0f, EqualTemperamentUtils::semitoneRatios(LYDIAN_STEPS)}},
        {"mixolydian", {2.0f, EqualTemperamentUtils::semitoneRatios(MIXOLYDIAN_STEPS)}},
        {"aeolian", {2.0f, EqualTemperamentUtils::semitoneRatios(AEOLIAN_STEPS)}},
        {"minor", {2.0f, EqualTemperamentUtils::semitoneRatios(AEOLIAN_STEPS)}},
        {"locrian", {2.0f, EqualTemperamentUtils::semitoneRatios(LOCRIAN_STEPS)}},

        // Minor variants
        {"harmonic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(HARMONIC_MINOR_STEPS)}},
        {"melodic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(MELODIC_MINOR_STEPS)}},

        // Pentatonic
        {"pentatonic major", {2.0f, EqualTemperamentUtils::semitoneRatios(MAJOR_PENTATONIC_STEPS)}},
        {"pentatonic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(MINOR_PENTATONIC_STEPS)}},

        // Blues
        {"blues", {2.0f, EqualTemperamentUtils::semitoneRatios(BLUES_STEPS)}},

        // Chromatic
        {"chromatic", {2.0f, EqualTemperamentUtils::semitoneRatios(CHROMATIC_STEPS)}},

        // Whole-tone
        {"whole-tone", {2.0f, EqualTemperamentUtils::semitoneRatios(WHOLE_TONE_STEPS)}},

        // Quarter-tone
        {"quarter-tone", {2.0f, EqualTemperamentUtils::semitoneRatios(QUARTER_TONE_STEPS, 24)}},

        // Microtonal / EDO
        {"edo19", {2.0f, EqualTemperamentUtils::semitoneRatios(EDO_19_STEPS, 19)}},

        // Tritave / tri-octave / Bohlen-Pierce
        {"bohlen-pierce", {3.0f, EqualTemperamentUtils::semitoneRatios(BOHLEN_PIERCE_STEPS, 13 * 3, 3.0f)}}
    };
} // p2t
