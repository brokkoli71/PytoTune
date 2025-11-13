//
// Created by Moritz Seppelt on 12.11.25.
//

#ifndef PYTOTUNE_MODETEMPLATES_H
#define PYTOTUNE_MODETEMPLATES_H
#include <vector>
#include <string>
#include <unordered_map>

namespace p2t {
    /**
     * Represents a musical mode, just like major, minor, or phrygian.
     * It is basically a scale without root note,
     * Just like our scale implementation, it is not restricted to any tuning system.
     */
    struct Mode {
        /**
         * The factor at which the scale repeats. Most likely it is 2, representing an octave.
         * It will always be greater than 1.
         */
        float repeatFactor;

        /**
         * The pitch proportions of the notes in the scale to the base note.
         * All values will always be between 1 and repeatFactor.
         * The vector is always sorted strictly increasingly.
         */
        std::vector<float> notes;
    };

    /**
     * Stores all modes referencable by name.
     */
    extern const std::unordered_map<std::string, Mode> modes;

    /**
     * Groups helper functions for scale creation, where the scale is based on equal divisions of some interval.
     * Fr example: This is useful to create equally tempered scales.
     */
    struct EqualTemperamentUtils {
        /**
         * @brief Computes the pitch ratio corresponding to a given number of semitone steps
         *        in an equal temperament system.
         *
         * This function calculates the frequency ratio for a note that is a specified number
         * of equal divisions above the base note, within a tuning system that repeats after
         * a defined interval (usually an octave, but may be another repeat factor such as 3
         * for Bohlen–Pierce scales).
         *
         * @param semitone       The number of semitone (or division) steps above the base note.
         *                       Can be negative, zero, or positive.
         * @param division       The total number of equal divisions in one repeat cycle
         *                       (e.g., 12 for 12-TET, 19 for 19-EDO, 24 for quarter-tone tuning).
         * @param repeatFactor   The frequency multiplication factor at which the scale repeats.
         *                       Commonly 2.0 for octaves, 3.0 for tritave systems, etc.
         *
         * @return The frequency ratio corresponding to the given semitone step.
         *         The result is clamped to the range [1.0f, repeatFactor].
         */

        static float semitoneRatio(int semitone, float division = 12.0f, float repeatFactor = 2.0f);

        /**
         * @brief Computes a list of pitch ratios for a set of semitone steps
         *        in an equal temperament (or equal division) tuning system.
         *
         * This function maps each semitone step in the provided vector to its
         * corresponding frequency ratio using the given division and repeat factor.
         * It generalizes to any equal division of a repeating interval — for example,
         * 12-TET (octave-based), 19-EDO, 24-EDO (quarter-tone), or 13-step tritave
         * systems like Bohlen–Pierce.
         *
         * @param semitones      A list of semitone (or division) step offsets from the base note.
         * @param division       The number of equal divisions within one repeat interval
         *                       (default: 12 for standard equal temperament).
         * @param repeatFactor   The frequency multiplication factor at which the scale repeats
         *                       (e.g., 2.0 for octaves, 3.0 for tritave systems).
         *
         * @return A vector of pitch ratios corresponding to the given semitone steps,
         *         each value clamped to the range [1.0f, repeatFactor].
         */
        static std::vector<float> semitoneRatios(const std::vector<int> &semitones, float division = 12.0f,
                                                 float repeatFactor = 2.0f);
    };
} //p2t

#endif //PYTOTUNE_MODETEMPLATES_H
