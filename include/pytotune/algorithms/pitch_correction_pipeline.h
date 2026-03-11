#ifndef PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#define PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
#include "yin_pitch_detector.h"
#include "pytotune/io/midi_file.h"
#include "pytotune/io/wav_file.h"
#include  "pytotune/data-structures/windowing.h"


namespace p2t {
#define DEFAULT_WINDOWING Windowing(4096, 4096 / 4)

    /**
     * End-to-end pitch correction pipeline for WAV input.
     *
     * The pipeline can either align detected pitch to MIDI notes or quantize
     * pitch to a given musical scale.
     */
    class PitchCorrectionPipeline {
    public:
        /**
         * Detect pitch per analysis window.
         *
         * This method is exposed separately for benchmarking and profiling.
         *
         * @param src Source WAV audio.
         * @param windowing Analysis window size and hop size.
         * @param pitchRange Minimum and maximum detectable pitch in Hz.
         * @param threshold YIN confidence threshold.
         * @param decimationFactor Downsampling factor used during detection.
         * @return Window-aligned detected pitch values in Hz.
         */
        WindowedData<float> detectPitch(const WavFile &src,
                                        Windowing windowing,
                                        PitchRange pitchRange,
                                        float threshold = DEFAULT_THRESHOLD,
                                        int decimationFactor = DEFAULT_DECIMATION_FACTOR) const;

        /**
         * Apply per-window correction factors and return pitch-shifted audio.
         *
         * This method is exposed separately for benchmarking and profiling.
         *
         * @param src Source WAV audio.
         * @param windowing Analysis window size and hop size.
         * @param correctionFactors Multiplicative pitch factors per window.
         * @return New WAV file with applied pitch shifts.
         */
        WavFile shiftPitch(const WavFile &src,
                           Windowing windowing,
                           const std::vector<float> &correctionFactors) const;

        /**
         * Tune audio to notes from a MIDI reference.
         *
         * @param src Source WAV audio.
         * @param midiFile MIDI note reference used as target pitch.
         * @param windowing Analysis window size and hop size.
         * @param tuning Reference tuning for A4 in Hz.
         * @param pitchRange Minimum and maximum detectable pitch in Hz.
         * @return New WAV file tuned to the MIDI reference.
         */
        WavFile matchMidi(const WavFile &src,
                          const MidiFile &midiFile,
                          Windowing windowing = DEFAULT_WINDOWING,
                          float tuning = DEFAULT_A4, PitchRange pitchRange = VoiceRanges::HUMAN) const;

        /**
         * Quantize detected pitch to the nearest notes of a musical scale.
         *
         * @param src Source WAV audio.
         * @param scale Target scale used for quantization.
         * @param windowing Analysis window size and hop size.
         * @param pitchRange Minimum and maximum detectable pitch in Hz.
         * @return New WAV file tuned to the provided scale.
         */
        WavFile roundToScale(const WavFile &src,
                             const Scale &scale,
                             Windowing windowing = DEFAULT_WINDOWING, PitchRange pitchRange = VoiceRanges::HUMAN) const;
    };
}

#endif //PYTOTUNE_PITCH_CORRECTION_PIPELINE_H
