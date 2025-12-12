//
// Created by Moritz Seppelt on 19.11.25.
//

#ifndef PYTOTUNE_FFT_H
#define PYTOTUNE_FFT_H

#include <vector>

namespace p2t {
    /**
     * @brief Safe atan2 function replacement to handle domain issues.
     *
     * @param x X-coordinate (horizontal component)
     * @param y Y-coordinate (vertical component)
     * @return angle in radians
     */
    double smbAtan2(double x, double y);

    /**
     * @brief Performs an in-place FFT or inverse FFT on a complex buffer.
     *
     * The fftBuffer is a vector of floats where real and imaginary parts are
     * interleaved: [Re0, Im0, Re1, Im1, ...].
     *
     * @param fftBuffer The interleaved complex buffer to transform
     * @param fftFrameSize Number of complex samples (must be a power of 2)
     * @param sign -1 for FFT, 1 for inverse FFT
     */
    void smbFft(std::vector<float> &fftBuffer, long fftFrameSize, long sign);
}

#endif //PYTOTUNE_FFT_H
