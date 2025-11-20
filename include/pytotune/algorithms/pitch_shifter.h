//
// Created by Moritz Seppelt on 19.11.25.
//

#ifndef PYTOTUNE_PITCH_SHIFTER_H
#define PYTOTUNE_PITCH_SHIFTER_H

#include <vector>

namespace p2t {
    /**
     * @class PitchShifter
     * @brief Implements a phase-vocoder pitch shifter.
     */
    class PitchShifter {
    public:
        /**
         * @brief Construct a PitchShifter.
         * @param fftSize FFT frame size (must be power of 2).
         * @param overlapFactor STFT oversampling factor.
         * @param sampleRate Sample rate in Hz.
         */
        PitchShifter(long fftSize, long overlapFactor, float sampleRate);

        /**
         * @brief Process a block of audio samples.
         * @param pitchShiftFactor Pitch shift multiplier (e.g., 2.0 = one octave up).
         * @param inData Input sample buffer.
         * @param outData Output sample buffer.
         * @param numSamples Number of samples to process.
         */
        void process(float pitchShiftFactor, const float *inData, float *outData, long numSamples);

    private:
        long fftSize; /**< FFT frame size */
        long halfSize; /**< Half of FFT size */
        long overlapFactor; /**< Oversampling factor */
        float sampleRate; /**< Sample rate in Hz */
        long stepSize; /**< Hop size = fftSize / overlapFactor */
        double freqPerBin; /**< Frequency per FFT bin */
        double expectedPhaseAdvance; /**< Expected phase advance per bin */
        long inputLatency; /**< Number of samples delay due to overlap */

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
    }
}
#endif //PYTOTUNE_PITCH_SHIFTER_H
