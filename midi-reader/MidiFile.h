//
// Created by Moritz Seppelt on 13.11.25.
//

#ifndef PYTOTUNE_MIDIFILE_H
#define PYTOTUNE_MIDIFILE_H
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct NoteEvent {
    uint8_t note; // MIDI note number
    float start; // start time in seconds
    float end; // end time in seconds
};

namespace p2t {
    class MidiFile {
    public:
        MidiFile() = default;

        /// Load and parse a MIDI file
        void load(const std::string &filename);

        /// Returns notes active at the given time (seconds)
        std::vector<int> getActiveNotesAt(float time) const;

        /// Total duration of the MIDI file in seconds
        float getLength() const { return lengthSeconds; }

    private:
        struct MidiHeader {
            uint16_t format;
            uint16_t numTracks;
            uint16_t division;
        };

        enum class EventType { NoteOn, NoteOff, Tempo, Other };

        struct MidiEvent {
            uint32_t absTicks;
            EventType type;
            uint8_t note; // for NoteOn/Off
            uint8_t velocity; // for NoteOn/Off
            uint32_t tempo; // for Tempo (µs per quarter note)
        };

        std::vector<NoteEvent> noteEvents;
        float lengthSeconds = 0.0f;

        // Helpers
        static uint16_t readUint16(std::ifstream &f);

        static uint32_t readUint32(std::ifstream &f);

        static uint32_t readVLQ(std::ifstream &f);

        MidiHeader readHeader(std::ifstream &f);

        std::vector<MidiEvent> readTrackEvents(std::ifstream &f, const MidiHeader &header);
    };
} // p2t

#endif //PYTOTUNE_MIDIFILE_H
