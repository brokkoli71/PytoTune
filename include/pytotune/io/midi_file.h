//
// Created by Moritz Seppelt on 13.11.25.
//

#ifndef PYTOTUNE_MIDIFILE_H
#define PYTOTUNE_MIDIFILE_H
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

/// Represents a single note event on the flattened timeline.
/// Track information is intentionally discarded: all tracks are merged.
///
/// - `note`  : MIDI note number (0–127)
/// - `start` : start time in seconds
/// - `end`   : end time in seconds
///
/// Important:
///   * Tempo changes affect **all tracks globally**.
///   * Notes from all tracks are **merged into one track** (track identity is not preserved).
struct NoteEvent {
    uint8_t note;
    float start;
    float end;
};

namespace p2t {
    class MidiFile {
    public:
        /// Load and parse a MIDI file into a flattened list of NoteEvents.
        ///
        /// Behaviour notes:
        ///   * All track events are combined into a single timeline.
        ///   * Tempo changes apply globally—no per-track tempo handling exists.
        ///   * Track numbers from the source file are read but ultimately ignored
        ///     once noteEvents are created.
        static MidiFile load(const std::string &filename);

        /// Returns all notes active at the given time (seconds).
        ///
        /// Since track information is discarded, this checks activity across
        /// the globally-merged timeline.
        std::vector<int> getActiveNotesAt(float time) const;

        std::vector<float> getActivePitchesAt(float time, float tuning) const;

        /// Total duration of the flattened MIDI in seconds.
        float getLength() const { return lengthSeconds; }

    private:
        MidiFile() = default;

        struct MidiHeader {
            uint16_t format;
            uint16_t numTracks;
            uint16_t division; // ticks per quarter note
        };

        enum class EventType { NoteOn, NoteOff, Tempo, Other };

        /// Internal representation of an event before flattening.
        /// Track number is preserved here for parsing, but is not used
        /// in the final noteEvents representation.
        struct MidiEvent {
            uint32_t absTicks; // absolute tick time
            EventType type;
            uint8_t note; // for NoteOn/NoteOff
            uint8_t velocity; // for NoteOn/NoteOff
            uint32_t tempo; // for Tempo (µs per quarter note)
            uint16_t track; // original track number
        };

        std::vector<NoteEvent> noteEvents;
        float lengthSeconds = 0.0f;

        // Helpers -------------------------------------------------------------

        static uint16_t readUint16(std::ifstream &f);

        static uint32_t readUint32(std::ifstream &f);

        static uint32_t readVLQ(std::ifstream &f);

        static MidiHeader readHeader(std::ifstream &f);

        /// Parse all events in a single MIDI track.
        /// Track index is provided so events can be tagged before merging.
        static std::vector<MidiEvent> readTrackEvents(std::ifstream &f,
                                                      const MidiHeader &header,
                                                      uint16_t track);
    };
} // p2t

#endif //PYTOTUNE_MIDIFILE_H
