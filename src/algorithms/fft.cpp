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

#define USE_PRECOMPUTED_TWIDDLES 1

#if USE_PRECOMPUTED_TWIDDLES
struct TwiddleTable {
    std::vector<float> re;
    std::vector<float> im;
    long size;

    TwiddleTable(long N, int sign) : size(N / 2), re(N / 2), im(N / 2) {
        for (long k = 0; k < size; ++k) {
            float angle = sign * 2.0f * M_PI * k / N;
            re[k] = std::cos(angle);
            im[k] = std::sin(angle);
        }
    }
};
#endif

namespace p2t {
    inline float smbAtan2(float x, float y) {
        float signx = (x > 0.0) ? 1.0 : -1.0;
        if (x == 0.0) return 0.0;
        if (y == 0.0) return signx * M_PI / 2.0;
        return std::atan2(x, y);
    }

    void smbFft(std::vector<float> &fftBuffer, long fftFrameSize, int sign) {
        if (fftFrameSize <= 0) return;

        const long N = fftFrameSize;
        if (fftBuffer.size() < 2 * N) return;

        // Check N is power of two, compute numStages
        long t = N;
        long numStages = 0;
        while (t > 1) {
            if (t % 2 != 0) return; // not power of two
            t >>= 1;
            ++numStages;
        }

        // Bit reversal on complex indices 0..N-1
        auto bit_reverse = [&](long x, long bits)-> long {
            long y = 0;
            for (long i = 0; i < bits; ++i) {
                y = (y << 1) | (x & 1);
                x >>= 1;
            }
            return y;
        };

#if USE_PRECOMPUTED_TWIDDLES
        TwiddleTable twiddles(N, sign); // precompute once for N
#endif

        long bits = numStages;
        for (long i = 0; i < N; ++i) {
            long j = bit_reverse(i, bits);
            if (j > i) {
                std::swap(fftBuffer[2 * i], fftBuffer[2 * j]);
                std::swap(fftBuffer[2 * i + 1], fftBuffer[2 * j + 1]);
            }
        }

        // Danielson-Lanczos / iterative radix-2
        // le = current DFT length in complex samples: 2,4,8,...,N
        for (long le = 2; le <= N; le <<= 1) {
            long le2 = le >> 1; // half size (butterfly distance)
            // compute basic angular increment for this stage
            // angle increment for k = 1: theta = sign * 2π / le
            float theta = sign * 2.0 * M_PI / static_cast<float>(le);

            // We will compute twiddles via recurrence to avoid many cos/sin calls.
            // For each k from 0..le2-1 compute W = exp(j * k * theta)
            for (long k = 0; k < le2; ++k) {
                // twiddle W = cos(k*theta) + j*sin(k*theta)
#if USE_PRECOMPUTED_TWIDDLES
                // pick correct stride for this stage
                long stride = N / le;
                float wr = twiddles.re[k * stride];
                float wi = twiddles.im[k * stride];
#else
                float wr = std::cos(k * theta);
                float wi = std::sin(k * theta);
#endif


                // Perform butterflies for this twiddle across all blocks
                for (long blockStart = 0; blockStart < N; blockStart += le) {
                    long i1 = blockStart + k; // index of upper complex sample
                    long i2 = i1 + le2; // index of lower complex sample

                    long p1 = 2 * i1;
                    long p1i = p1 + 1;
                    long p2 = 2 * i2;
                    long p2i = p2 + 1;

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
