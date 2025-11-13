//
// Created by Moritz Seppelt on 12.11.25.
//

#ifndef PYTOTUNE_SCALE_H
#define PYTOTUNE_SCALE_H

#define DEFAULT_A4 442
#include <vector>

#include "Mode.h"

namespace p2t {
    /**
     * Represents a musical scale, just like C minor or Eb major.
     * It is not restricted a any particular tuning.
     */
    class Scale {
    private:
        /**
         * The pitch of the base/root note of the scale.
         * It will always have a positive value.
         */
        float baseNote;

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

        /**
         * Validates the correctness of the parameters. Those have to match the following requirements:
         * @param baseNote Must be positive
         * @param repeatFactor Must be greater than 1.
         * @param notes Must be sorter strictly ascendingly. All notes must be between 1 and the repeatFactor.
         *
         * @throws std::invalid_argument If any of the requirements ontop are not matched.
         */
        static void validate(float baseNote, float repeatFactor, const std::vector<float> &notes);

    public:
        /**
         * Constructor of the Scale class.
         * @param baseNote The pitch of the base/root note of the scale. Must be positive.
         * @param repeatFactor The factor at which the scale repeats. Most likely it is 2, representing an octave. Must be greater than 1.
         * @param notes The pitch proportions of the notes in the scale to the base note. Must be sorter strictly ascendingly. All notes must be between 1 and the repeatFactor
         *
         * @throws std::invalid_argument If any of the requirements ontop are not matched.
         */
        Scale(float baseNote, float repeatFactor, std::vector<float> notes);

        /**
         * Given a pitch of some note, the closest note in the scale is calculated.
         * @param pitch The pitch of the nate to be rounded.
         * @return The closest pitch in the scale.
         */
        float getClosestPitchInScale(float pitch) const;

        // Getters and Setters
        float getBaseNote() const;

        /**
         * Sets the base note.
         * @param newBaseNote Must be  positive.
         * @throws std::invalid_argument If newBaseNote is not positive.
         */
        void setBaseNote(float newBaseNote);


        float getRepeatFactor() const;

        /**
         * Sets the repeat factor.
         * @param newRepeatFactor Must be greater than 1.
         * @throws std::invalid_argument If newRepeatFactor is less than 1.
         */
        void setRepeatFactor(float newRepeatFactor);

        const std::vector<float> &getNotes() const;

        /**
         * Sets the notes of the scale.
         * @param newNotes Must be sorter strictly ascendingly. All notes must be between 1 and the repeatFactor.
         * @throws std::invalid_argument If newNotes do not match its requirements.
         */
        void setNotes(std::vector<float> newNotes);

        // --------------- Alternate Constructors ----------------
        /**
         * Alternate constructor for the Scale class.
         * This is useful to create scales by simple language, like "Eb major", "G blues" or "chromatic".
         *
         * @param name The natural language name of the scale. It can have 2 different formats:
         * - <atonal scale> The atonal scale can be one of "chromatic", "whole-tone", "quarter-tone"
         * - <root note> <scale> The root note has the format [A|B|C|D|E|F|G](#|b)?, e.g., A, Cb, F#. The scale can be one of "major just", "minor just", "ionian", "major", "dorian", "phrygian", "lydian", "mixolydian", "aeolian", "minor", "locrian", "harmonic minor", "melodic minor", "pentatonic major", "pentatonic minor", "blues", "chromatic", "whole-tone", "quarter-tone", "edo19", "bohlen-pierce"
         * @param tuning Sets the pitch of A4. Usually, it is set to a number close to 440 Hz.
         * @return The scale that corresponds to the name.
         * @throws std::invalid_argument if the name doesn't match its requirements.
         */
        static Scale fromName(const std::string &name, float tuning = DEFAULT_A4);

        /**
         * Alternate constructor for the Scale class.
         * Returns a scale based on a mode name and a root note.
         *
         * @param modeName The name of the mode, can be one of "major just", "minor just", "ionian", "major", "dorian", "phrygian", "lydian", "mixolydian", "aeolian", "minor", "locrian", "harmonic minor", "melodic minor", "pentatonic major", "pentatonic minor", "blues", "chromatic", "whole-tone", "quarter-tone", "edo19", "bohlen-pierce"
         * @param baseNote Sets the pitch of the base/root note of the scale.
         * @return The scale that is created using the mode refered by the modeName and the baseNote.
         * @throws std::invalid_argument if the mode name is not supported.
         */
        static Scale fromModeName(const std::string &modeName, float baseNote);

        /**
         * Alternate constructor for the Scale class.
         * Returns a scale based on a mode and a root note.
         *
         * @param mode The mode the scale is created from.
         * @param baseNote The pitch base/root note of the resulting scale.
         * @return The scale created from the Mode.
         */
        static Scale fromMode(const Mode &mode, float baseNote);
    };
} // namespace p2t

#endif  // PYTOTUNE_SCALE_H
