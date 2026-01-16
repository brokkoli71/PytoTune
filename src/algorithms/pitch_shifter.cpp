//
// Created by Moritz Seppelt on 02.12.25.
//
#include "../../include/pytotune/algorithms/pitch_shifter.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "pytotune/algorithms/fft.h"

namespace p2t {
    std::vector<float> PitchShifter::run(const std::vector<float> &samples, float pitchFactor) const {
        return run(samples, {windowing, std::vector<float>(samples.size() / windowing.stride, pitchFactor)});
    }

    std::vector<float> PitchShifter::run(const std::vector<float> &samples,
                                         const WindowedData<float> &pitchFactors) const {
        const float maxPitchFactor = *std::ranges::max_element(pitchFactors.data);
        const int buffer_size = static_cast<int>(std::pow(2, std::ceil(std::log2(maxPitchFactor)))) * 2 * windowing.
                                windowSize;

        std::vector<float> inFifo(buffer_size, 0.0f);
        std::vector<float> outFifo(buffer_size, 0.0f);
        std::vector<float> fftWorkspace(2 * buffer_size, 0.0f);
        std::vector<float> lastPhase(buffer_size / 2 + 1, 0.0f);
        std::vector<float> sumPhase(buffer_size / 2 + 1, 0.0f);
        std::vector<float> outputAccum(2 * buffer_size, 0.0f);
        std::vector<float> anaFreq(buffer_size, 0.0f);
        std::vector<float> anaMagn(buffer_size, 0.0f);

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
                    float window = -.5f * cos(2. * M_PI * (double) k / (double) windowing.windowSize) + .5;
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
                    tmp -= (double) k * expect;

                    /* map delta phase into +/- Pi interval */
                    int qpd = tmp / M_PI;
                    if (qpd >= 0)
                        qpd += qpd & 1;
                    else
                        qpd -= qpd & 1;
                    tmp -= M_PI * (double) qpd;

                    /* get deviation from bin frequency from the +/- Pi interval */
                    tmp = windowing.getOsamp() * tmp / (2. * M_PI);

                    /* compute the k-th partials' true frequency */
                    tmp = (double) k * freqPerBin + tmp * freqPerBin;

                    /* store magnitude and true frequency in analysis arrays */
                    anaMagn[k] = magn;
                    anaFreq[k] = tmp;
                }

                /* ***************** PROCESSING ******************* */
                /* this does the actual pitch shifting */
                std::vector<float> gSynFreq(buffer_size, 0.0f);
                std::vector<float> gSynMagn(buffer_size, 0.0f);
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
                    tmp -= (double) k * freqPerBin;

                    /* get bin deviation from freq deviation */
                    tmp /= freqPerBin;

                    /* take osamp into account */
                    tmp = 2. * M_PI * tmp / windowing.getOsamp();

                    /* add the overlap phase advance back in */
                    tmp += (double) k * expect;

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
                    float window = -.5 * cos(2. * M_PI * (double) k / (double) windowing.windowSize) + .5;
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
    }
} // namespace p2t
