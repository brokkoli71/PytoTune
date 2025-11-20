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
    inline double smbAtan2(double x, double y) {
        double signx = (x > 0.0) ? 1.0 : -1.0;
        if (x == 0.0) return 0.0;
        if (y == 0.0) return signx * M_PI / 2.0;
        return std::atan2(x, y);
    }

    void smbFft(std::vector<float> &fftBuffer, long fftFrameSize, long sign) {
        // Bit-reversal permutation
        for (long i = 2; i < 2 * fftFrameSize - 2; i += 2) {
            long j = 0;
            for (long bitm = 2; bitm < 2 * fftFrameSize; bitm <<= 1) {
                if (i & bitm) j++;
                j <<= 1;
            }
            if (i < j) {
                std::swap(fftBuffer[i], fftBuffer[j]);
                std::swap(fftBuffer[i + 1], fftBuffer[j + 1]);
            }
        }

        long numStages = static_cast<long>(std::log2(fftFrameSize) + 0.5);
        for (long k = 0, le = 2; k < numStages; ++k) {
            le <<= 1;
            long le2 = le >> 1;

            double ur = 1.0, ui = 0.0;
            double arg = M_PI / (le2 >> 1);
            double wr = std::cos(arg);
            double wi = sign * std::sin(arg);

            for (long j = 0; j < le2; j += 2) {
                for (long i = j; i < 2 * fftFrameSize; i += le) {
                    long p1r = i;
                    long p1i = i + 1;
                    long p2r = i + le2;
                    long p2i = i + le2 + 1;

                    double tr = fftBuffer[p2r] * ur - fftBuffer[p2i] * ui;
                    double ti = fftBuffer[p2r] * ui + fftBuffer[p2i] * ur;

                    fftBuffer[p2r] = fftBuffer[p1r] - tr;
                    fftBuffer[p2i] = fftBuffer[p1i] - ti;

                    fftBuffer[p1r] += tr;
                    fftBuffer[p1i] += ti;
                }
                double tr = ur * wr - ui * wi;
                ui = ur * wi + ui * wr;
                ur = tr;
            }
        }
    }
} // namespace BernseeFFT

