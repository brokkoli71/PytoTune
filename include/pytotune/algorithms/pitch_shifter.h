//
// Created by Moritz Seppelt on 19.11.25.
//

#ifndef PYTOTUNE_PITCH_SHIFTER_H
#define PYTOTUNE_PITCH_SHIFTER_H

#include <vector>

#include "pytotune/yin_pitch_detector.h"
#include "pytotune/data-structures/scale.h"
#include "pytotune/io/midi_file.h"

namespace p2t {
    /**
     * @class PitchShifter
     * @brief Implements a phase-vocoder pitch shifter.
     */
    class PitchShifter {
    public:
        /**
         * @brief Construct a PitchShifter.
         * @param windowSize FFT frame size (must be power of 2).
         * @param overlapFactor STFT oversampling factor.
         * @param sampleRate Sample rate in Hz.
         * @param windowPitchFactors The pitch correction factors per window
         */
        PitchShifter(int windowSize, int overlapFactor, float sampleRate, std::vector<float> windowPitchFactors);

        static PitchShifter createPitchRounder(const PitchDetection &pitchDetection, const Scale &scale);

        static PitchShifter createMidiMatcher(const PitchDetection &pitchDetection, const MidiFile &midiFile,
                                              float tuning = DEFAULT_A4);

        static PitchShifter createConstantPitchMatcher(const PitchDetection &pitchDetection, float pitch);

        /**
         * @brief Process a block of audio samples.
         * @param pitchShiftFactor Pitch shift multiplier (e.g., 2.0 = one octave up).
         * @param inData Input sample buffer.
         * @param outData Output sample buffer.
         * @param numSamples Number of samples to process.
         */
        void process(const float *inData, float *outData, long numSamples);

    private:
        int windowSize; /**< FFT frame size */
        int overlapFactor; /**< Oversampling factor */
        float sampleRate; /**< Sample rate in Hz */
        int stepSize; /**< Hop size = fftSize / overlapFactor */
        double freqPerBin; /**< Frequency per FFT bin */
        double expectedPhaseAdvance; /**< Expected phase advance per bin */
        int inputLatency; /**< Number of samples delay due to overlap */
        std::vector<float> windowPitchFactors;

        bool init; /**< Buffers initialized flag */
        long rover; /**< Current position in input/output FIFO */

        std::vector<float> inputFIFO; /**< Input circular buffer */
        std::vector<float> outputFIFO; /**< Output circular buffer */
        std::vector<float> fftWorkspace; /**< Interleaved real/imag buffer */
        std::vector<double> lastPhase; /**< Last frame phases */
        std::vector<double> sumPhase; /**< Accumulated phases */
        std::vector<float> outputAccumulator; /**< Overlap-add accumulator */
        std::vector<float> analysisMagnitude; /**< Analysis magnitudes */
        std::vector<float> analysisFrequency; /**< Analysis frequencies */
        std::vector<float> synthesisMagnitude; /**< Synthesis magnitudes */
        std::vector<float> synthesisFrequency; /**< Synthesis frequencies */

        /** Initialize all working buffers */
        void initializeBuffers();

        /** Apply Hann window and prepare FFT input buffer */
        void applyWindowToInput();

        /** Analyze FFT spectrum: magnitude and true frequency via phase difference */
        void analyzeSpectrum();

        /** Pitch-shift by re-mapping magnitudes and frequencies */
        void pitchShiftSpectrum(float pitchShiftFactor);

        /** Synthesize signal from shifted spectrum */
        void synthesizeSignal();

        /** Apply window, scale, and overlap-add to output */
        void overlapAddOutput();

        /** Shift output accumulator left by stepSize */
        void shiftAccumulators();

        /** Shift input FIFO left to make room for next frame */
        void shiftInputFIFO();
    };
}
#endif //PYTOTUNE_PITCH_SHIFTER_H
