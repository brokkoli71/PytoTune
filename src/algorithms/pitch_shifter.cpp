/****************************************************************************
*
* This code is inspired by Stephan M. Bernsee's smbPitchShift algorithm
* (http://blogs.zynaptiq.com/bernsee) and uses some of the original ideas.
* All implementation and modernization here are heavily adapted and written
* in modern C++ style.
*
* Original source copyright 1999-2015 Stephan M. Bernsee.
* Used for reference only; the current code is independently developed.
*
****************************************************************************/

#include "pytotune/algorithms/pitch_shifter.h"
#include "pytotune/algorithms/fft.h"
#include <utility>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace p2t {
    constexpr int MAX_FRAME_LENGTH = 8192;

    PitchShifter::PitchShifter(int windowSize, int overlapFactor, float sampleRate,
                               std::vector<float> windowPitchFactors)
        : windowSize(windowSize),
          overlapFactor(overlapFactor),
          sampleRate(sampleRate),
          stepSize(windowSize / overlapFactor),
          freqPerBin(sampleRate / static_cast<float>(windowSize)),
          expectedPhaseAdvance(2.0 * M_PI * static_cast<double>(stepSize) / static_cast<double>(windowSize)),
          inputLatency(windowSize - stepSize),
          init(false),
          rover(0),
          windowPitchFactors(std::move(windowPitchFactors)) {
        initializeBuffers();
        rover = inputLatency;
    }

    PitchShifter PitchShifter::createPitchRounder(const PitchDetection &pitchDetection, const Scale &scale) {
        std::vector<float> windowPitchFactors;
        windowPitchFactors.reserve(pitchDetection.pitch_values.size());

        for (const float detectedPitch: pitchDetection.pitch_values) {
            const float targetPitch = scale.getClosestPitchInScale(detectedPitch);
            windowPitchFactors.push_back(targetPitch / detectedPitch);
        }

        const int overlapFactor = static_cast<int>(
            1.0f / (1.0f - static_cast<float>(pitchDetection.window_overlap) / static_cast<float>(pitchDetection
                        .window_size)));
        return {
            pitchDetection.window_size,
            overlapFactor,
            static_cast<float>(pitchDetection.audio_buffer->sampleRate),
            windowPitchFactors
        };
    }

    PitchShifter PitchShifter::createMidiMatcher(const PitchDetection &pitchDetection, const MidiFile &midiFile,
                                                 const float tuning) {
        const int stepSize = pitchDetection.window_size - pitchDetection.window_overlap;
        std::vector<float> windowPitchFactors;
        windowPitchFactors.reserve(pitchDetection.pitch_values.size());
        float sampleRate = static_cast<float>(pitchDetection.audio_buffer->sampleRate);

        const int overlapFactor = static_cast<int>(
            1.0f / (1.0f - static_cast<float>(pitchDetection.window_overlap) / static_cast<float>(pitchDetection
                        .window_size)));

        for (int i = 0; i < pitchDetection.pitch_values.size(); i++) {
            const float time = static_cast<float>(i * stepSize) / sampleRate;
            const std::vector<float> targetPitches = midiFile.getActivePitchesAt(time, tuning);

            // TODO: how to get target pitch
            const float targetPitch = targetPitches.empty() ? 0 : targetPitches.front();

            windowPitchFactors.push_back(targetPitch / pitchDetection.pitch_values[i]);
        }

        return {
            pitchDetection.window_size,
            overlapFactor,
            sampleRate,
            windowPitchFactors
        };
    }

    PitchShifter PitchShifter::createConstantPitchMatcher(const PitchDetection &pitchDetection, float pitch) {
        std::vector<float> windowPitchFactors;
        windowPitchFactors.reserve(pitchDetection.pitch_values.size());

        for (const float detectedPitch: pitchDetection.pitch_values) {
            windowPitchFactors.push_back(pitch / detectedPitch);
        }

        const int overlapFactor = static_cast<int>(
            1.0f / (1.0f - static_cast<float>(pitchDetection.window_overlap) / static_cast<float>(pitchDetection
                        .window_size)));

        return {
            pitchDetection.window_size,
            overlapFactor,
            static_cast<float>(pitchDetection.audio_buffer->sampleRate),
            windowPitchFactors
        };
    }


    // -----------------------------------------------------------
    // Process a block of audio samples
    // inData  -> input samples
    // outData -> output samples
    // numSamples -> number of samples to process
    // pitchShiftFactor -> e.g. 2.0 = one octave up, 0.5 = one octave down
    // -----------------------------------------------------------
    void PitchShifter::process(const float *inData,
                               float *outData,
                               long numSamples) {
        long windowNum = 0;
        for (long i = 0; i < numSamples; ++i) {
            // -----------------------------------------------------------------
            // Fill input circular buffer and output any previously accumulated output
            // -----------------------------------------------------------------
            inputFIFO[rover] = inData[i];
            outData[i] = outputFIFO[rover - inputLatency];
            rover++;

            // -----------------------------------------------------------------
            // When one FFT window is collected, process it
            // -----------------------------------------------------------------
            if (rover >= windowSize) {
                rover = inputLatency;

                applyWindowToInput();
                smbFft(fftWorkspace, windowSize, -1); // Forward FFT

                analyzeSpectrum();
                pitchShiftSpectrum(windowPitchFactors[windowNum]);
                synthesizeSignal();

                smbFft(fftWorkspace, windowSize, 1); // Inverse FFT
                overlapAddOutput();

                shiftAccumulators();
                shiftInputFIFO();
                windowNum++;
            }
        }
    }

    void PitchShifter::initializeBuffers() {
        inputFIFO.assign(MAX_FRAME_LENGTH, 0.0f);
        outputFIFO.assign(MAX_FRAME_LENGTH, 0.0f);
        fftWorkspace.assign(2 * MAX_FRAME_LENGTH, 0.0f);
        lastPhase.assign(MAX_FRAME_LENGTH / 2 + 1, 0.0);
        sumPhase.assign(MAX_FRAME_LENGTH / 2 + 1, 0.0);
        outputAccumulator.assign(2 * MAX_FRAME_LENGTH, 0.0f);
        analysisMagnitude.assign(MAX_FRAME_LENGTH, 0.0f);
        analysisFrequency.assign(MAX_FRAME_LENGTH, 0.0f);
        synthesisMagnitude.assign(MAX_FRAME_LENGTH, 0.0f);
        synthesisFrequency.assign(MAX_FRAME_LENGTH, 0.0f);
        init = true;
    }

    void PitchShifter::applyWindowToInput() {
        for (long k = 0; k < windowSize; ++k) {
            double window = -0.5 * std::cos(2.0 * M_PI * k / windowSize) + 0.5;
            fftWorkspace[2 * k] = inputFIFO[k] * window;
            fftWorkspace[2 * k + 1] = 0.0f;
        }
    }

    void PitchShifter::analyzeSpectrum() {
        for (long k = 0; k <= windowSize / 2; ++k) {
            double real = fftWorkspace[2 * k];
            double imag = fftWorkspace[2 * k + 1];

            double magnitude = 2.0 * std::sqrt(real * real + imag * imag);
            double phase = std::atan2(imag, real);

            double phaseDiff = phase - lastPhase[k];
            lastPhase[k] = phase;

            // Remove expected phase advance for a steady sinusoid
            phaseDiff -= static_cast<double>(k) * expectedPhaseAdvance;

            // Wrap to -PI..PI
            long qpd = static_cast<long>(phaseDiff / M_PI);
            if (qpd >= 0) qpd += qpd & 1;
            else qpd -= qpd & 1;
            phaseDiff -= static_cast<double>(qpd) * M_PI;

            // Convert delta-phase to deviation from bin frequency
            phaseDiff = overlapFactor * phaseDiff / (2.0 * M_PI);

            double trueFreq = static_cast<double>(k) * freqPerBin + phaseDiff * freqPerBin;

            analysisMagnitude[k] = static_cast<float>(magnitude);
            analysisFrequency[k] = static_cast<float>(trueFreq);
        }
    }

    void PitchShifter::pitchShiftSpectrum(float pitchShiftFactor) {
        std::fill(synthesisMagnitude.begin(), synthesisMagnitude.end(), 0.0f);
        std::fill(synthesisFrequency.begin(), synthesisFrequency.end(), 0.0f);

        for (long k = 0; k <= windowSize / 2; ++k) {
            long newIndex = static_cast<long>(k * pitchShiftFactor);
            if (newIndex <= windowSize / 2) {
                synthesisMagnitude[newIndex] += analysisMagnitude[k];
                synthesisFrequency[newIndex] = analysisFrequency[k] * pitchShiftFactor;
            }
        }
    }

    void PitchShifter::synthesizeSignal() {
        for (long k = 0; k <= windowSize / 2; ++k) {
            double magnitude = synthesisMagnitude[k];
            double trueFreq = synthesisFrequency[k];

            // Frequency offset relative to FFT bin
            double freqOffset = trueFreq - (static_cast<double>(k) * freqPerBin);
            freqOffset /= freqPerBin;

            // Phase advance by this offset
            double phaseIncrement = 2.0 * M_PI * freqOffset / overlapFactor;
            phaseIncrement += static_cast<double>(k) * expectedPhaseAdvance;

            // Accumulate phase
            sumPhase[k] += phaseIncrement;
            double phase = sumPhase[k];

            fftWorkspace[2 * k] = static_cast<float>(magnitude * std::cos(phase));
            fftWorkspace[2 * k + 1] = static_cast<float>(magnitude * std::sin(phase));
        }

        // Zero out upper bins
        for (long k = windowSize + 2; k < 2 * windowSize; ++k) {
            fftWorkspace[k] = 0.0f;
        }
    }

    // ---------------------------------------------------------------
    // Apply window, scale, and add to output accumulator
    // ---------------------------------------------------------------
    void PitchShifter::overlapAddOutput() {
        for (long k = 0; k < windowSize; ++k) {
            double window = -0.5 * std::cos(2.0 * M_PI * k / windowSize) + 0.5;
            outputAccumulator[k] += static_cast<float>(
                2.0 * window * fftWorkspace[2 * k] / (windowSize / 2 * overlapFactor)
            );
        }

        for (long k = 0; k < stepSize; ++k) {
            outputFIFO[k] = outputAccumulator[k];
        }
    }

    // ---------------------------------------------------------------
    // Shift accumulator left by stepSize
    // ---------------------------------------------------------------
    void PitchShifter::shiftAccumulators() {
        std::memmove(outputAccumulator.data(),
                     outputAccumulator.data() + stepSize,
                     windowSize * sizeof(float));

        // Zero the end region
        std::fill(outputAccumulator.begin() + windowSize,
                  outputAccumulator.end(),
                  0.0f);
    }

    // ---------------------------------------------------------------
    // Shift input FIFO left to make room for next frame
    // ---------------------------------------------------------------
    void PitchShifter::shiftInputFIFO() {
        for (long k = 0; k < inputLatency; ++k) {
            inputFIFO[k] = inputFIFO[k + stepSize];
        }
    }
};

