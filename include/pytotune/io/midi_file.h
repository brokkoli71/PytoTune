#ifndef PYTOTUNE_MIDIFILE_H
#define PYTOTUNE_MIDIFILE_H
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "pytotune/data-structures/scale.h"
#include "pytotune/data-structures/windowing.h"

/**
 * Represents a single note event on the flattened MIDI timeline.
 *
 * Track information is intentionally discarded after parsing: notes from all
 * tracks are merged and interpreted on one global timeline.
 */
struct NoteEvent {
    /// MIDI note number in the range [0, 127].
    uint8_t note;
    /// Note start time in seconds.
    float start;
    /// Note end time in seconds.
    float end;
};

namespace p2t {
    class MidiFile {
    public:
        /**
         * Load and parse a MIDI file into a flattened list of note events.
         *
         * All track events are combined into one timeline. Tempo changes are
         * interpreted globally, matching standard MIDI tempo semantics.
         *
         * @param filename Path to the input MIDI file.
         * @return Parsed MIDI object with flattened note events.
         */
        static MidiFile load(const std::string &filename);

        /**
         * Get all notes active at the given time.
         * @param time Query time in seconds.
         * @return MIDI note numbers active at the query time.
         */
        std::vector<int> getActiveNotesAt(float time) const;

        /**
         * Get all active pitches (Hz) at the given time.
         * @param time Query time in seconds.
         * @param tuning Reference tuning for A4 in Hz.
         * @return Active pitches in Hz at the query time.
         */
        std::vector<float> getActivePitchesAt(float time, float tuning = DEFAULT_A4) const;

        /**
         * Sample active notes at window start times.
         * @param windowing Window size and hop size used for sampling.
         * @param sampleRate Audio sample rate in Hz.
         * @return Windowed active-note vectors.
         */
        WindowedData<std::vector<int> > getWindowedNotes(const Windowing &windowing, float sampleRate) const;

        /**
         * Sample the highest active note at window start times.
         * @param windowing Window size and hop size used for sampling.
         * @param sampleRate Audio sample rate in Hz.
         * @param defaultNote Value used when no note is active in a window.
         * @return Windowed highest-note values.
         */
        WindowedData<int> getWindowedHighestNotes(const Windowing &windowing, float sampleRate,
                                                  int defaultNote = 0) const;

        /**
         * Sample active pitches at window start times.
         * @param windowing Window size and hop size used for sampling.
         * @param sampleRate Audio sample rate in Hz.
         * @param tuning Reference tuning for A4 in Hz.
         * @return Windowed active-pitch vectors in Hz.
         */
        WindowedData<std::vector<float> > getWindowedPitches(const Windowing &windowing, float sampleRate,
                                                             float tuning = DEFAULT_A4) const;

        /**
         * Sample the highest active pitch at window start times.
         * @param windowing Window size and hop size used for sampling.
         * @param sampleRate Audio sample rate in Hz.
         * @param defaultPitch Value used when no note is active in a window.
         * @param tuning Reference tuning for A4 in Hz.
         * @return Windowed highest-pitch values in Hz.
         */
        WindowedData<float> getWindowedHighestPitches(const Windowing &windowing, float sampleRate,
                                                      float defaultPitch = 0.0f,
                                                      float tuning = DEFAULT_A4) const;

        /**
         * Computes pitch correction factors to match detected pitches to the MIDI targets.
         * @param pitches The detected pitches per window (Hz). A pitch of 0 yields a correction factor of 1.
         * @param windowing The windowing used during pitch detection.
         * @param sampleRate The sample rate of the audio.
         * @param tuning The tuning of A4 in Hz.
         * @return A vector of correction factors, one per window.
         */
        std::vector<float> getPitchCorrectionFactors(const WindowedData<float> &pitches,
                                                     const Windowing &windowing,
                                                     float sampleRate,
                                                     float tuning = DEFAULT_A4) const;

        /**
         * Convert a MIDI note number to frequency in Hz.
         * @param note MIDI note number.
         * @param tuning Reference tuning for A4 in Hz.
         * @return Frequency in Hz.
         */
        inline static float noteToPitch(int note, float tuning = DEFAULT_A4);

        /**
         * Get total duration of the flattened MIDI timeline.
         * @return Duration in seconds.
         */
        float getLength() const { return lengthSeconds; }

    private:
        MidiFile() = default;

        struct MidiHeader {
            uint16_t format;
            uint16_t numTracks;
            uint16_t division; // ticks per quarter note
        };

        enum class EventType {
            NoteOn,
            NoteOff,
            Tempo,
            Other
        };

        /**
         * Internal event representation used during MIDI parsing.
         *
         * Track number is preserved at this stage and discarded after
         * flattening to `NoteEvent`.
         */
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

        /**
         * Parse all events from a single MIDI track chunk.
         * @param f Input stream positioned at the track chunk payload.
         * @param header Parsed MIDI header.
         * @param track Track index used to tag parsed events.
         * @return Parsed events for the given track.
         */
        static std::vector<MidiEvent> readTrackEvents(std::ifstream &f,
                                                      const MidiHeader &header,
                                                      uint16_t track);
    };
} // namespace p2t

#endif  // PYTOTUNE_MIDIFILE_H
