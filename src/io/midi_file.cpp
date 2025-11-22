//
// Created by Moritz Seppelt on 13.11.25.
//

#include "pytotune/io/midi_file.h"

#include <algorithm>
#include <fstream>
#include <ranges>
#include <stdexcept>
#include <cmath>

namespace p2t {
    uint16_t MidiFile::readUint16(std::ifstream &f) {
        uint8_t b[2];
        f.read(reinterpret_cast<char *>(b), 2);
        return (b[0] << 8) | b[1];
    }

    uint32_t MidiFile::readUint32(std::ifstream &f) {
        uint8_t b[4];
        f.read(reinterpret_cast<char *>(b), 4);
        return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    }

    uint32_t MidiFile::readVLQ(std::ifstream &f) {
        uint32_t value = 0;
        uint8_t byte;
        do {
            f.read(reinterpret_cast<char *>(&byte), 1);
            value = (value << 7) | (byte & 0x7F);
        } while (byte & 0x80);
        return value;
    }

    MidiFile::MidiHeader MidiFile::readHeader(std::ifstream &f) {
        char id[4];
        f.read(id, 4);
        if (std::string(id, 4) != "MThd") throw std::runtime_error("Not a MIDI file");

        uint32_t length = readUint32(f);
        if (length < 6) throw std::runtime_error("Invalid header length");

        MidiHeader header{};
        header.format = readUint16(f);
        header.numTracks = readUint16(f);
        header.division = readUint16(f);

        f.seekg(length - 6, std::ios::cur);
        return header;
    }

    // ---------------------- Track parsing ----------------------
    std::vector<MidiFile::MidiEvent> MidiFile::readTrackEvents(std::ifstream &f, const MidiHeader &header,
                                                               const uint16_t track) {
        char id[4];
        f.read(id, 4);
        if (std::string(id, 4) != "MTrk") throw std::runtime_error("Missing track");

        uint32_t length = readUint32(f);
        std::streampos trackEnd = f.tellg() + static_cast<std::streampos>(length);

        std::vector<MidiEvent> events;
        uint8_t runningStatus = 0;
        uint32_t absTicks = 0;

        while (f.tellg() < trackEnd) {
            uint32_t deltaTicks = readVLQ(f);
            absTicks += deltaTicks;

            uint8_t statusByte;
            f.read(reinterpret_cast<char *>(&statusByte), 1);
            if (statusByte < 0x80) {
                f.unget();
                statusByte = runningStatus;
            } else {
                runningStatus = statusByte;
            }

            if ((statusByte & 0xF0) == 0x90) {
                // Note On
                uint8_t note, vel;
                f.read(reinterpret_cast<char *>(&note), 1);
                f.read(reinterpret_cast<char *>(&vel), 1);
                events.push_back({absTicks, vel > 0 ? EventType::NoteOn : EventType::NoteOff, note, vel, 0, track});
            } else if ((statusByte & 0xF0) == 0x80) {
                // Note Off
                uint8_t note, vel;
                f.read(reinterpret_cast<char *>(&note), 1);
                f.read(reinterpret_cast<char *>(&vel), 1);
                events.push_back({absTicks, EventType::NoteOff, note, vel, 0, track});
            } else if (statusByte == 0xFF) {
                // Meta
                uint8_t type;
                f.read(reinterpret_cast<char *>(&type), 1);
                uint32_t len = readVLQ(f);
                if (type == 0x51 && len == 3) {
                    // Tempo
                    uint8_t t[3];
                    f.read(reinterpret_cast<char *>(t), 3);
                    uint32_t tempo = (t[0] << 16) | (t[1] << 8) | t[2];
                    events.push_back({absTicks, EventType::Tempo, 0, 0, tempo, track});
                } else {
                    f.seekg(len, std::ios::cur);
                }
            } else if (statusByte == 0xF0 || statusByte == 0xF7) {
                // SysEx
                uint32_t len = readVLQ(f);
                f.seekg(len, std::ios::cur);
            } else {
                int paramBytes = ((statusByte & 0xF0) == 0xC0 || (statusByte & 0xF0) == 0xD0) ? 1 : 2;
                f.seekg(paramBytes, std::ios::cur);
            }
        }

        return events;
    }

    // ---------------------- Load MIDI ----------------------
    MidiFile MidiFile::load(const std::string &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f) throw std::runtime_error("Failed to open file");

        MidiHeader header = readHeader(f);

        // --- PASS 1: Read all tracks into absolute-tick events ---
        std::vector<MidiEvent> allEvents;
        for (int i = 0; i < header.numTracks; i++) {
            auto trackEvents = readTrackEvents(f, header, i);
            allEvents.insert(allEvents.end(), trackEvents.begin(), trackEvents.end());
        }

        // Sort the events
        std::sort(allEvents.begin(), allEvents.end(), [](const MidiEvent &a, const MidiEvent &b) {
            if (a.absTicks != b.absTicks)
                return a.absTicks < b.absTicks;

            // For same absTicks, NoteOff comes before NoteOn
            if ((a.type == EventType::NoteOff) && (b.type == EventType::NoteOn))
                return true;
            if ((a.type == EventType::NoteOn) && (b.type == EventType::NoteOff))
                return false;

            // fallback to original order for other cases
            return false;
        });

        // --- PASS 2: Sort by absTicks and convert to seconds ---
        // activeNotes[track][noteValue] = noteOnEventTime
        std::unordered_map<uint16_t, std::unordered_map<uint8_t, float> > activeNotes;

        std::vector<NoteEvent> noteEvents;
        noteEvents.reserve(allEvents.size() / 2);

        double tempo = 500000.0; // default 120 BPM
        uint32_t lastTick = 0;
        double currentTime = 0.0;

        for (auto &e: allEvents) {
            double deltaTicks = e.absTicks - lastTick;
            double secondsPerTick = tempo / 1e6 / header.division;
            currentTime += deltaTicks * secondsPerTick;
            lastTick = e.absTicks;

            if (e.type == EventType::Tempo) {
                tempo = e.tempo;
            } else if (e.type == EventType::NoteOn) {
                activeNotes[e.track][e.note] = static_cast<float>(currentTime);
            } else if (e.type == EventType::NoteOff) {
                if (activeNotes[e.track].contains(e.note) && activeNotes[e.track][e.note] < currentTime) {
                    noteEvents.push_back({e.note, activeNotes[e.track][e.note], static_cast<float>(currentTime)});
                    activeNotes[e.track].erase(e.note);
                }
            }
        }

        float lengthSeconds = static_cast<float>(currentTime);

        // Remaining notes still "on"
        for (auto &val: activeNotes | std::views::values) {
            for (auto &notesMap = val; auto &[note, value]: notesMap) {
                noteEvents.push_back({note, value, lengthSeconds});
            }
        }

        // Sort them by the start
        std::sort(noteEvents.begin(), noteEvents.end(),
                  [](const NoteEvent &a, const NoteEvent &b) {
                      return a.start < b.start; // compare startTime
                  });
        MidiFile result;
        result.noteEvents = noteEvents;
        result.lengthSeconds = lengthSeconds;

        return result;
    }

    // ---------------------- Query ----------------------
    std::vector<int> MidiFile::getActiveNotesAt(const float time) const {
        if (time > lengthSeconds || time < 0) return {};

        std::vector<int> result;
        for (const auto &n: noteEvents) {
            if (n.start <= time && n.end > time)
                result.push_back(n.note);

            // Exit early because it is sorted by the start time
            if (n.start > time)
                return result;
        }

        return result;
    }

    std::vector<float> MidiFile::getActivePitchesAt(const float time, const float tuning) const {
        constexpr int a4 = 69;
        const std::vector<int> notes = getActiveNotesAt(time);
        std::vector<float> pitches(notes.size());

        std::transform(notes.begin(), notes.end(), pitches.begin(), [&tuning](int note) {
            return tuning * std::pow(2.0f, static_cast<float>(note - a4) / 12.0f);
        });

        return pitches;
    }
} // namespace p2t
