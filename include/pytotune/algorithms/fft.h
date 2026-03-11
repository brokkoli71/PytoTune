#ifndef PYTOTUNE_FFT_H
#define PYTOTUNE_FFT_H

#include <vector>

namespace p2t {
    /**
     * Performs an in-place FFT or inverse FFT on a complex buffer.
     *
     * The fftBuffer is a vector of floats where real and imaginary parts are
     * interleaved: [Re0, Im0, Re1, Im1, ...].
     *
     * @param fftBuffer The interleaved complex buffer to transform
     * @param fftFrameSize Number of complex samples (must be a power of 2)
     * @param sign -1 for FFT, 1 for inverse FFT
     */
    void smbFft(std::vector<float> &fftBuffer, int fftFrameSize, int sign);
}

#endif //PYTOTUNE_FFT_H
