#define _USE_MATH_DEFINES  // Ensure M_PI is defined on MSVC

#include "../../include/pytotune/algorithms/pitch_shifter.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "pytotune/algorithms/fft.h"

#define REIMPLEMENTED_WINDOWING 1

namespace p2t {
std::vector<float> PitchShifter::run(const std::vector<float> &samples, const WindowedData<float> &pitchFactors,
                                     bool limitToOctave) const {
    std::cout << "Min Factor: " << *std::ranges::min_element(pitchFactors.data) << std::endl;
    std::cout << "Max Factor: " << *std::ranges::max_element(pitchFactors.data) << std::endl;

    std::vector<float> remaining = pitchFactors.data;
    std::vector<float> audio = samples;

    if (limitToOctave) {
        for (float &pitchFactor : remaining) {
            if (pitchFactor > 2.f)
                pitchFactor /= std::exp2(std::floor(std::log2(pitchFactor)));

            if (pitchFactor < 0.5f)
                pitchFactor *= std::exp2(std::floor(std::log2(1.f / pitchFactor)));
        }
    }

    std::cout << "Min Factor: " << *std::ranges::min_element(remaining) << std::endl;
    std::cout << "Max Factor: " << *std::ranges::max_element(remaining) << std::endl;

    bool done = false;
    while (!done) {
        done = true;
        std::vector<float> step = remaining;

        for (unsigned int i = 0; i < remaining.size(); ++i) {
            if (remaining[i] > 2.0f) {
                step[i] = 2.0f;
                remaining[i] /= 2.0f;
                done = false;
            } else if (remaining[i] < 0.5f) {
                step[i] = 0.5f;
                remaining[i] *= 2.f;
                done = false;
            } else {
                step[i] = remaining[i];
                remaining[i] = 1.0f;
            }
        }

        audio = runWithClampedPitchFactors(audio, {pitchFactors.windowing, step});
    }

    return audio;
}

std::vector<float> PitchShifter::run(const std::vector<float> &samples, float pitchFactor) const {
    return run(samples, {windowing, std::vector<float>(samples.size() / windowing.stride, pitchFactor)});
}

std::vector<float> PitchShifter::runWithClampedPitchFactors(const std::vector<float> &samples,
                                                            const WindowedData<float> &pitchFactors) const {
    // We are expecting all factors to be in [0.5, 2.0]

    const int bufferSize = 2 * windowing.windowSize;
    const int numWindows = samples.size() / windowing.stride;
#ifdef REIMPLEMENTED_WINDOWING
    std::vector<float> lastPhase(bufferSize / 2 + 1, 0.0f);
    std::vector<float> sumPhase(bufferSize / 2 + 1, 0.0f);
    std::vector<std::vector<float> > fftWorkspace(numWindows, std::vector<float>(2 * bufferSize));
    std::vector<std::vector<float> > anaFreq(
        numWindows, std::vector<float>(bufferSize));

    std::vector<std::vector<float> > anaMagn(
        numWindows, std::vector<float>(bufferSize));
    std::vector<float> outData(samples.size(), 0.0f);

    const float expect = 2.f * static_cast<float>(M_PI) * static_cast<float>(windowing.stride) / static_cast<float>(windowing.windowSize);
    const float freqPerBin = sampleRate / static_cast<float>(windowing.windowSize);

#pragma omp parallel for ordered
    for (int windowIndex = 0; windowIndex < numWindows; ++windowIndex) {
        /* do windowing and re,im interleave */
        for (int k = 0; k < windowing.windowSize; k++) {
            if (windowIndex * windowing.stride + k >= samples.size()) break;
            const float window = -.5f * std::cos(
                                            2.f * static_cast<float>(M_PI) * static_cast<float>(k) / static_cast<float>(windowing.windowSize)) +
                                 .5f;
            fftWorkspace[windowIndex][2 * k] = samples[windowIndex * windowing.stride + k] * window;
            fftWorkspace[windowIndex][2 * k + 1] = 0.;
        }

        /* ***************** ANALYSIS ******************* */
        /* do transform */
        p2t::smbFft(fftWorkspace[windowIndex], windowing.windowSize, -1);

#pragma omp ordered
        {
            for (int k = 0; k <= windowing.windowSize; k++) {
                /* de-interlace FFT buffer */
                const float real = fftWorkspace[windowIndex][2 * k];
                const float imag = fftWorkspace[windowIndex][2 * k + 1];

                /* compute magnitude and phase */
                float magn = 2. * sqrt(real * real + imag * imag);
                float phase = atan2(imag, real);

                /* compute phase difference */
                float tmp = phase - lastPhase[k];
                lastPhase[k] = phase;

                /* subtract expected phase difference */
                tmp -= static_cast<float>(k) * expect;

                /* map delta phase into +/- Pi interval */
                int qpd = tmp / M_PI;
                if (qpd >= 0)
                    qpd += qpd & 1;
                else
                    qpd -= qpd & 1;
                tmp -= M_PI * (double)qpd;

                /* get deviation from bin frequency from the +/- Pi interval */
                tmp = static_cast<float>(windowing.getOsamp()) * tmp / static_cast<float>(2. * M_PI);

                /* compute the k-th partials' true frequency */
                tmp = static_cast<float>(k) * freqPerBin + tmp * freqPerBin;

                /* store magnitude and true frequency in analysis arrays */
                anaMagn[windowIndex][k] = magn;
                anaFreq[windowIndex][k] = tmp;
            }
        }

        /* ***************** PROCESSING ******************* */
        /* this does the actual pitch shifting */
        std::vector<float> gSynFreq(bufferSize, 0.0f);
        std::vector<float> gSynMagn(bufferSize, 0.0f);
        const float factor = pitchFactors.data[windowIndex];
        for (int k = 0; k <= bufferSize / 2; k++) {
            int index = static_cast<int>(std::round(k * factor));
            ;
            gSynMagn[index] += anaMagn[windowIndex][k];
            gSynFreq[index] = anaFreq[windowIndex][k] * factor;
        }

        /* ***************** SYNTHESIS ******************* */
        /* this is the synthesis step */
        for (int k = 0; k <= windowing.windowSize; k++) {
            /* get magnitude and true frequency from synthesis arrays */
            float magn = gSynMagn[k];
            float tmp = gSynFreq[k];

            /* subtract bin mid frequency */
            tmp -= (double)k * freqPerBin;

            /* get bin deviation from freq deviation */
            tmp /= freqPerBin;

            /* take osamp into account */
            tmp = static_cast<float>(2. * M_PI) * tmp / static_cast<float>(windowing.getOsamp());

            /* add the overlap phase advance back in */
            tmp += static_cast<float>(k) * expect;

            /* accumulate delta phase to get bin phase */
            sumPhase[k] += tmp;
            float phase = sumPhase[k];

            /* get real and imag part and re-interleave */
            fftWorkspace[windowIndex][2 * k] = magn * std::cos(phase);
            fftWorkspace[windowIndex][2 * k + 1] = magn * std::sin(phase);
        }

        /* zero negative frequencies */
        for (int k = windowing.windowSize + 2; k < 2 * windowing.windowSize; k++)
            fftWorkspace[windowIndex][k] = 0.;

        /* do inverse transform */
        smbFft(fftWorkspace[windowIndex], windowing.windowSize, 1);

        for (int k = 0; k < windowing.windowSize; ++k) {
            if (windowIndex * windowing.stride + k >= samples.size()) break;
            float window =
                -0.5f * std::cos(2.0f * M_PI * k / windowing.windowSize) + 0.5f;

#pragma omp atomic update
            outData[windowIndex * windowing.stride + k] +=
                2.0f * window *
                fftWorkspace[windowIndex][2 * k] /
                static_cast<float>(windowing.windowSize / 2 * windowing.getOsamp());
        }
    }

    return outData;

#else
    std::vector<float> inFifo(bufferSize, 0.0f);
    std::vector<float> outFifo(bufferSize, 0.0f);
    std::vector<float> fftWorkspace(2 * bufferSize, 0.0f);
    std::vector<float> lastPhase(bufferSize / 2 + 1, 0.0f);
    std::vector<float> sumPhase(bufferSize / 2 + 1, 0.0f);
    std::vector<float> outputAccum(2 * bufferSize, 0.0f);
    std::vector<float> anaFreq(bufferSize, 0.0f);
    std::vector<float> anaMagn(bufferSize, 0.0f);

    std::vector<float> outData(samples.size(), 0.0f);

    /* set up some handy variables */
    int fftFrameSize2 = windowing.windowSize / 2;
    float freqPerBin = sampleRate / static_cast<float>(windowing.windowSize);
    float expect = 2.0f * M_PI * static_cast<float>(windowing.stride) / static_cast<float>(windowing.windowSize);
    int inFifoLatency = windowing.windowSize - windowing.stride;
    int rover = inFifoLatency;
    int windowIndex = 0;

    /* main processing loop */
    for (int i = 0; i < samples.size(); i++) {
        /* As long as we have not yet collected enough data just read in */
        inFifo[rover] = samples[i];
        outData[i] = outFifo[rover - inFifoLatency];
        rover++;

        /* now we have enough data for processing */
        if (rover >= windowing.windowSize) {
            rover = inFifoLatency;

            /* do windowing and re,im interleave */
            for (int k = 0; k < windowing.windowSize; k++) {
                float window = -.5f * cos(2. * M_PI * (double)k / (double)windowing.windowSize) + .5;
                fftWorkspace[2 * k] = inFifo[k] * window;
                fftWorkspace[2 * k + 1] = 0.;
            }

            /* ***************** ANALYSIS ******************* */
            /* do transform */
            p2t::smbFft(fftWorkspace, windowing.windowSize, -1);

            /* this is the analysis step */
            for (int k = 0; k <= fftFrameSize2; k++) {
                /* de-interlace FFT buffer */
                float real = fftWorkspace[2 * k];
                float imag = fftWorkspace[2 * k + 1];

                /* compute magnitude and phase */
                float magn = 2. * sqrt(real * real + imag * imag);
                float phase = atan2(imag, real);

                /* compute phase difference */
                float tmp = phase - lastPhase[k];
                lastPhase[k] = phase;

                /* subtract expected phase difference */
                tmp -= (double)k * expect;

                /* map delta phase into +/- Pi interval */
                int qpd = tmp / M_PI;
                if (qpd >= 0)
                    qpd += qpd & 1;
                else
                    qpd -= qpd & 1;
                tmp -= M_PI * (double)qpd;

                /* get deviation from bin frequency from the +/- Pi interval */
                tmp = windowing.getOsamp() * tmp / (2. * M_PI);

                /* compute the k-th partials' true frequency */
                tmp = (double)k * freqPerBin + tmp * freqPerBin;

                /* store magnitude and true frequency in analysis arrays */
                anaMagn[k] = magn;
                anaFreq[k] = tmp;
            }

            /* ***************** PROCESSING ******************* */
            /* this does the actual pitch shifting */
            std::vector<float> gSynFreq(bufferSize, 0.0f);
            std::vector<float> gSynMagn(bufferSize, 0.0f);
            float factor = pitchFactors.data[windowIndex];
            for (int k = 0; k <= fftFrameSize2; k++) {
                int index = k * factor;
                if (index <= fftFrameSize2) {
                    gSynMagn[index] += anaMagn[k];
                    gSynFreq[index] = anaFreq[k] * factor;
                }
            }

            /* ***************** SYNTHESIS ******************* */
            /* this is the synthesis step */
            for (int k = 0; k <= fftFrameSize2; k++) {
                /* get magnitude and true frequency from synthesis arrays */
                float magn = gSynMagn[k];
                float tmp = gSynFreq[k];

                /* subtract bin mid frequency */
                tmp -= (double)k * freqPerBin;

                /* get bin deviation from freq deviation */
                tmp /= freqPerBin;

                /* take osamp into account */
                tmp = 2. * M_PI * tmp / windowing.getOsamp();

                /* add the overlap phase advance back in */
                tmp += (double)k * expect;

                /* accumulate delta phase to get bin phase */
                sumPhase[k] += tmp;
                float phase = sumPhase[k];

                /* get real and imag part and re-interleave */
                fftWorkspace[2 * k] = magn * cos(phase);
                fftWorkspace[2 * k + 1] = magn * sin(phase);
            }

            /* zero negative frequencies */
            for (int k = windowing.windowSize + 2; k < 2 * windowing.windowSize; k++) fftWorkspace[k] = 0.;

            /* do inverse transform */
            smbFft(fftWorkspace, windowing.windowSize, 1);

            /* do windowing and add to output accumulator */
            for (int k = 0; k < windowing.windowSize; k++) {
                float window = -.5 * cos(2. * M_PI * (double)k / (double)windowing.windowSize) + .5;
                outputAccum[k] += 2. * window * fftWorkspace[2 * k] / (fftFrameSize2 * windowing.getOsamp());
            }
            for (int k = 0; k < windowing.stride; k++) outFifo[k] = outputAccum[k];

            /* shift accumulator */
            std::move(
                outputAccum.begin() + windowing.stride,
                outputAccum.begin() + windowing.stride + windowing.windowSize,
                outputAccum.begin());
            std::fill_n(
                outputAccum.begin() + windowing.windowSize,
                windowing.stride,
                0.0f);

            /* move input FIFO */
            for (int k = 0; k < inFifoLatency; k++) inFifo[k] = inFifo[k + windowing.stride];

            windowIndex++;
        }
    }

    return outData;
#endif
}
}  // namespace p2t
