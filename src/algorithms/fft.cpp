#define _USE_MATH_DEFINES // Ensure M_PI is defined on MSVC
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

#include "pytotune/algorithms/fft.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace p2t {
    void smbFft(std::vector<float> &fftBuffer, const int fftFrameSize, const int sign) {
        if (fftFrameSize <= 0) return;
        const int N = fftFrameSize;
        if (fftBuffer.size() < 2 * N) return;

        // Check N is power of two, compute numStages
        int t = N;
        int numStages = 0;
        while (t > 1) {
            if (t % 2 != 0) return; // not power of two
            t >>= 1;
            ++numStages;
        }

        // Bit reversal on complex indices 0..N-1
        auto bit_reverse = [&](int x, const int bits)-> int {
            int y = 0;
            for (int i = 0; i < bits; ++i) {
                y = (y << 1) | (x & 1);
                x >>= 1;
            }
            return y;
        };

        int const bits = numStages;
        for (int i = 0; i < N; ++i) {
            int j = bit_reverse(i, bits);
            if (j > i) {
                std::swap(fftBuffer[2 * i], fftBuffer[2 * j]);
                std::swap(fftBuffer[2 * i + 1], fftBuffer[2 * j + 1]);
            }
        }

        // Danielson-Lanczos / iterative radix-2
        // le = current DFT length in complex samples: 2,4,8,...,N
        for (int le = 2; le <= N; le <<= 1) {
            int le2 = le >> 1; // half size (butterfly distance)
            // compute basic angular increment for this stage
            // angle increment for k = 1: theta = sign * 2π / le
            float theta = static_cast<float>(sign) * 2.f * static_cast<float>(M_PI) / static_cast<float>(le);

            // We will compute twiddles via recurrence to avoid many cos/sin calls.
            // For each k from 0..le2-1 compute W = exp(j * k * theta)
            for (int k = 0; k < le2; ++k) {
                // twiddle W = cos(k*theta) + j*sin(k*theta)
                const float wr = std::cos(static_cast<float>(k) * theta);
                const float wi = std::sin(static_cast<float>(k) * theta);

                // Perform butterflies for this twiddle across all blocks
                for (int blockStart = 0; blockStart < N; blockStart += le) {
                    int i1 = blockStart + k; // index of upper complex sample
                    int i2 = i1 + le2; // index of lower complex sample

                    int p1 = 2 * i1;
                    int p1i = p1 + 1;
                    int p2 = 2 * i2;
                    int p2i = p2 + 1;

                    float r1 = fftBuffer[p1];
                    float i1v = fftBuffer[p1i];
                    float r2 = fftBuffer[p2];
                    float i2v = fftBuffer[p2i];

                    // t = W * lower
                    float tr = r2 * wr - i2v * wi;
                    float ti = r2 * wi + i2v * wr;

                    // lower = upper - t
                    fftBuffer[p2] = r1 - tr;
                    fftBuffer[p2i] = i1v - ti;

                    // upper = upper + t
                    fftBuffer[p1] = r1 + tr;
                    fftBuffer[p1i] = i1v + ti;
                }
            }
        }
    }
} // namespace BernseeFFT
