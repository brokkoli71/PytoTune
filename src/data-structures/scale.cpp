//
// Created by Moritz Seppelt on 12.11.25.
//

#include <utility>
#include <cmath>
#include <stdexcept>

#include "pytotune/data-structures/scale.h"

#include <unordered_map>

namespace p2t {
    Scale::Scale(float baseNote, float repeatFactor, std::vector<float> notes) {
        validate(baseNote, repeatFactor, notes);
        this->baseNote = baseNote;
        this->repeatFactor = repeatFactor;
        this->notes = std::move(notes);
    }

    void Scale::validate(float baseNote, float repeatFactor, const std::vector<float> &notes) {
        if (baseNote <= 0.0f)
            throw std::invalid_argument("baseNote must be positive.");

        if (repeatFactor < 1.0f)
            throw std::invalid_argument("repeatFactor must be greater than 1.");

        if (notes.empty())
            throw std::invalid_argument("Scale notes cannot be empty.");

        for (std::size_t i = 0; i < notes.size(); ++i) {
            if (notes[i] < 1.0f)
                throw std::invalid_argument("Scale notes must be greater than 1.");
            if (notes[i] >= repeatFactor)
                throw std::invalid_argument("Scale notes must be less than repeatFactor.");
            if (i > 0 && notes[i] <= notes[i - 1])
                throw std::invalid_argument("Scale notes must be strictly ascending.");
        }
    }


    float Scale::getClosestPitchInScale(float pitch) const {
        // We are looking for a, where pitch = a * basePitch
        const float a = pitch / this->baseNote;

        // But a cannot be bigger than repeatFactor, because then we are in a different octave
        // So we are looking for the octave number n, where a = repeatFactor^n * r (where 1 <= r < repeatFactor)
        // log(a) = n * log(repeatFactor) + log(r)                      (*)
        // n = log(a) / log(repeatFactor) - log(r) / log(repeatFactor)
        // Because 1 <= r < repeatFactor, we get that 0 <= log(r) / log(repeatFactor) < 1, this
        // n = floor(log(a) / log(repeatFactor))
        const float n = std::floor(std::log(a) / std::log(this->repeatFactor));

        // We can arrange (*) to
        // log(r) = log(a) - n * log(repeatFactor)
        const float logR = std::log(a) - n * std::log(this->repeatFactor);
        const float r = std::exp(logR);

        // Now we will find the notes in the scale r lies between, by a quick linear search
        // Note: Binary search would be unappropriated, because there in every practical application less than 100 entries
        int upperIndex = 0;

        for (; upperIndex < this->notes.size(); upperIndex++) {
            if (this->notes[upperIndex] > r)
                break;
        }

        const float lowerNote = upperIndex == 0
                                    ? (this->notes.back() / this->repeatFactor)
                                    : this->notes[upperIndex - 1];
        const float upperNote = upperIndex == this->notes.size()
                                    ? (this->notes[0] * this->repeatFactor)
                                    : this->notes[upperIndex];

        // Now we know which note it lies between
        // To now decide which one the pitch is closest to, we will compare the logs, as the pitch scale is logarithmic
        const float distToLower = logR - std::log(lowerNote);
        const float distToUpper = std::log(upperNote) - logR;
        const float closestNote = distToLower < distToUpper ? lowerNote : upperNote;


        // Don't forget to readd the octave we calculated
        return closestNote * this->baseNote * std::pow(this->repeatFactor, n);
    }


    // Getters and setters
    float Scale::getBaseNote() const {
        return baseNote;
    }

    void Scale::setBaseNote(float newBaseNote) {
        validate(newBaseNote, this->repeatFactor, this->notes);
        this->baseNote = newBaseNote;
    }

    float Scale::getRepeatFactor() const {
        return repeatFactor;
    }

    void Scale::setRepeatFactor(float newRepeatFactor) {
        validate(this->baseNote, newRepeatFactor, this->notes);
        this->repeatFactor = newRepeatFactor;
    }

    const std::vector<float> &Scale::getNotes() const {
        return notes;
    }

    void Scale::setNotes(std::vector<float> newNotes) {
        validate(this->baseNote, this->repeatFactor, newNotes);
        this->notes = std::move(newNotes);
    }

    // --------------- Alternate Constructors ----------------
    Scale Scale::fromName(const std::string &name, float tuning) {
        // Also accept atonal scales, that dont need a root note
        const std::vector<std::string> atonalScales{
            "chromatic", "whole-tone", "quarter-tone"
        };

        for (const std::string &atonalScale: atonalScales) {
            if (name == atonalScale)
                return Scale::fromModeName(name, tuning);
        }


        // Split name at first space
        const std::size_t pos = name.find(' '); // find first space
        if (pos == std::string::npos)
            throw std::invalid_argument(
                "Invalid scale name format. The expected format is: "
                "<root note> <mode name>, where the root note is one or two letters (e.g., C, Bb), "
                "followed by a space, and then the mode name, which may contain spaces (e.g., 'major', 'minor pentatonic'). "
                "Example: 'Bb major', 'C# lydian', 'F minor pentatonic'."
            );

        const std::string note = name.substr(0, pos);
        const std::string modeName = name.substr(pos + 1);

        // Define semitones
        static const std::unordered_map<std::string, int> noteSemitones = {
            {"Cb", 2}, {"C", 3}, {"C#", 4},
            {"Db", 4}, {"D", 5}, {"D#", 6},
            {"Eb", 6}, {"E", 7}, {"E#", 8},
            {"Fb", 7}, {"F", 8}, {"F#", 9},
            {"Gb", 9}, {"G", 10}, {"G#", 11},
            {"Ab", 11}, {"A", 0}, {"A#", 1},
            {"Bb", 1}, {"B", 2}, {"B#", 3}
        };

        if (noteSemitones.count(note) == 0)
            throw std::invalid_argument("Invalid root note. It mus have the format: [A|B|C|D|E|F|G](#|b)?");

        return Scale::fromModeName(
            modeName, tuning * std::pow(2.0f, static_cast<float>(noteSemitones.at(note)) / 12.0f));
    }

    Scale Scale::fromModeName(const std::string &modeName, float baseNote) {
        if (modes.count(modeName) == 0)
            throw std::invalid_argument("Unknown mode name '" + modeName + "'");

        return Scale::fromMode(modes.at(modeName), baseNote);
    }

    Scale Scale::fromMode(const Mode &mode, float baseNote) {
        return {baseNote, mode.repeatFactor, mode.notes};
    }
} // namespace p2t
