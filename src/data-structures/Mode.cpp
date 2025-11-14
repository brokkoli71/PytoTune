//
// Created by Moritz Seppelt on 12.11.25.
//

#include "pytotune/data-structures/Mode.h"
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
    static const std::vector<float> justMajor = {1.0f, 9.0f / 8, 5.0f / 4, 4.0f / 3, 3.0f / 2, 5.0f / 3, 15.0f / 8};
    static const std::vector<float> justMinor = {1.0f, 9.0f / 8, 6.0f / 5, 4.0f / 3, 3.0f / 2, 8.0f / 5, 9.0f / 5};

    // ---------------------- Equal temperament diatonic modes ----------------------
    static const std::vector<int> ionianSteps = {0, 2, 4, 5, 7, 9, 11}; // Major
    static const std::vector<int> dorianSteps = {0, 2, 3, 5, 7, 9, 10};
    static const std::vector<int> phrygianSteps = {0, 1, 3, 5, 7, 8, 10};
    static const std::vector<int> lydianSteps = {0, 2, 4, 6, 7, 9, 11};
    static const std::vector<int> mixolydianSteps = {0, 2, 4, 5, 7, 9, 10};
    static const std::vector<int> aeolianSteps = {0, 2, 3, 5, 7, 8, 10}; // Minor
    static const std::vector<int> locrianSteps = {0, 1, 3, 5, 6, 8, 10};

    // More minor variants
    static const std::vector<int> harmonicMinSteps = {0, 2, 3, 5, 7, 8, 11};
    static const std::vector<int> melodicMinSteps = {0, 2, 3, 5, 7, 9, 11};

    // Chromatic scale (12-TET)
    static const std::vector<int> chromaticSteps = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    // Whole tone scale (6-TET)
    static const std::vector<int> wholeToneSteps = {0, 2, 4, 6, 8, 10};

    // Quarter tone scale (24-TET)
    static const std::vector<int> quarterToneSteps = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
    };

    // Pentatonics
    static const std::vector<int> majorPentatonicSteps = {0, 2, 4, 7, 9};
    static const std::vector<int> minorPentatonicSteps = {0, 3, 5, 7, 10};

    // Blues scale
    static const std::vector<int> bluesSteps = {0, 3, 5, 6, 7, 10};

    // ---------------------- Microtonal / non-octave ----------------------
    // Example: 19-EDO
    static const std::vector<int> edo19Steps = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};

    // Bohlen-Pierce / tritave scale
    static const std::vector<int> bohlenPierceSteps = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36};

    // ---------------------- Modes map ----------------------
    const std::unordered_map<std::string, Mode> modes = {
        // Just intonation
        {"major just", {2.0f, justMajor}},
        {"minor just", {2.0f, justMinor}},

        // Equal temperament diatonic
        {"ionian", {2.0f, EqualTemperamentUtils::semitoneRatios(ionianSteps)}},
        {"major", {2.0f, EqualTemperamentUtils::semitoneRatios(ionianSteps)}},
        {"dorian", {2.0f, EqualTemperamentUtils::semitoneRatios(dorianSteps)}},
        {"phrygian", {2.0f, EqualTemperamentUtils::semitoneRatios(phrygianSteps)}},
        {"lydian", {2.0f, EqualTemperamentUtils::semitoneRatios(lydianSteps)}},
        {"mixolydian", {2.0f, EqualTemperamentUtils::semitoneRatios(mixolydianSteps)}},
        {"aeolian", {2.0f, EqualTemperamentUtils::semitoneRatios(aeolianSteps)}},
        {"minor", {2.0f, EqualTemperamentUtils::semitoneRatios(aeolianSteps)}},
        {"locrian", {2.0f, EqualTemperamentUtils::semitoneRatios(locrianSteps)}},

        // Minor variants
        {"harmonic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(harmonicMinSteps)}},
        {"melodic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(melodicMinSteps)}},

        // Pentatonic
        {"pentatonic major", {2.0f, EqualTemperamentUtils::semitoneRatios(majorPentatonicSteps)}},
        {"pentatonic minor", {2.0f, EqualTemperamentUtils::semitoneRatios(minorPentatonicSteps)}},

        // Blues
        {"blues", {2.0f, EqualTemperamentUtils::semitoneRatios(bluesSteps)}},

        // Chromatic
        {"chromatic", {2.0f, EqualTemperamentUtils::semitoneRatios(chromaticSteps)}},

        // Whole-tone
        {"whole-tone", {2.0f, EqualTemperamentUtils::semitoneRatios(wholeToneSteps)}},

        // Quarter-tone
        {"quarter-tone", {2.0f, EqualTemperamentUtils::semitoneRatios(quarterToneSteps, 24)}},

        // Microtonal / EDO
        {"edo19", {2.0f, EqualTemperamentUtils::semitoneRatios(edo19Steps, 19)}},

        // Tritave / tri-octave / Bohlen-Pierce
        {"bohlen-pierce", {3.0f, EqualTemperamentUtils::semitoneRatios(bohlenPierceSteps, 13 * 3, 3.0f)}}
    };
} // p2t
