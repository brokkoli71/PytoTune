//
// Created by Moritz Seppelt on 12.11.25.
//

#ifndef PYTOTUNE_SCALE_H
#define PYTOTUNE_SCALE_H

#define DEFAULT_A4 442
#include <vector>

#include "ModeTemplates.h"

namespace p2t {

class Scale {

private:

    // The pitch of the base not of the scale.
    float baseNote;

    // The factor at which the scale repeats. Most likely it is 2, representing an octave.
    float repeatFactor;

    // The pitch proportions of the notes in the scale to the base note
    std::vector<float> notes;

    // Static validation helper
    static void validate(float baseNote, float repeatFactor, const std::vector<float>& notes);

public:
    Scale(float baseNote, float repeatFactor, std::vector<float> notes);

    float getClosestPitchInScale(float pitch) const;

    // Getters and Setters
    float getBaseNote() const;
    void setBaseNote(float newBaseNote);

    float getRepeatFactor() const;
    void setRepeatFactor(float newRepeatFactor);

    const std::vector<float>& getNotes() const;
    void setNotes(std::vector<float> newNotes);

    // Alternate constructors
    static Scale fromName(const std::string& name, float tuning = DEFAULT_A4);
    static Scale fromModeName(const std::string& modeName, float baseNote);
    static Scale fromMode(const Mode& mode, float baseNote);



};

}  // namespace p2t

#endif  // PYTOTUNE_SCALE_H
