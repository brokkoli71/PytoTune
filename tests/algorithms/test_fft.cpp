#include <cmath>
#include <gtest/gtest.h>
#include <vector>
#include "pytotune/algorithms/fft.h"
#include "../test_utils.h"


TEST(SmbFftTest, NoOpForZeros) {
    int n = 4;
    std::vector<float> buf(2 * n, 0.0f);

    p2t::smbFft(buf, n, +1);

    // A transform of zeros should remain zeros.
    EXPECT_NEAR_VEC(buf, std::vector<float>(2 * n));
}

TEST(SmbFftTest, ImpulseProducesFlatSpectrum) {
    long N = 4;
    std::vector<float> buf(2 * N, 0.0f);

    // Real impulse at index 0
    buf[0] = 1.0f; // real part
    buf[1] = 0.0f; // imag part

    p2t::smbFft(buf, N, -1);

    std::vector<float> expected = {
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 0.0f,
    };

    EXPECT_NEAR_VEC(buf, expected);
}

TEST(SmbFftTest, SimpleKnownTransformSize4) {
    long N = 4;

    // Input: real sequence [1, 2, 3, 4], imag = 0
    // Layout: [r0,i0, r1,i1, r2,i2, r3,i3]
    std::vector<float> buf = {1, 0, 2, 0, 3, 0, 4, 0};

    p2t::smbFft(buf, N, -1);

    // Expected forward FFT of [1,2,3,4]:
    // k=0: 10 + 0i
    // k=1: -2 + 2i
    // k=2: -2 + 0i
    // k=3: -2 - 2i
    std::vector<float> expected = {
        10, 0, -2, 2, -2, 0, -2, -2
    };

    EXPECT_NEAR_VEC(buf, expected);
}

TEST(SmbFftTest, InverseReturnsOriginalForSize4) {
    long N = 4;
    std::vector<float> original = {1, 0, 2, 0, 3, 0, 4, 0};
    std::vector<float> buf = original;

    p2t::smbFft(buf, N, -1); // forward
    p2t::smbFft(buf, N, +1); // inverse

    // The algorithm does not scale by 1/N on inverse;
    // you typically need to divide by N afterwards.
    for (auto &v: buf) v /= N;

    EXPECT_NEAR_VEC(buf, original);
}

TEST(SmbFftTest, HandlesNonTrivialImagParts) {
    long N = 4;
    std::vector<float> buf = {
        1, 1, // 1 + i
        0, -1, // -i
        -1, 0, // -1
        0, 1 // i
    };

    // Just checking the transform is consistent with
    // a forward then inverse round-trip.
    auto orig = buf;

    p2t::smbFft(buf, N, +1);
    p2t::smbFft(buf, N, -1);
    for (auto &v: buf) v /= N;

    EXPECT_NEAR_VEC(buf, orig);
}

TEST(SmbFftTest, PerfectSingleBinSpectrum) {
    const int N = 1024; // FFT size
    const float fs = 48000.0f;

    // Choose a bin that matches perfectly
    const int targetBin = 10;
    const float freq = targetBin * fs / N; // exactly bin-aligned!

    const float twoPi = 6.283185307f;

    // Allocate 2*N floats (real/imag interleaved)
    std::vector<float> buf(2 * N, 0.0f);

    // Generate x[n] = sin(2π * targetBin * n / N)
    // This creates an *exact* integer number of cycles (targetBin cycles).
    for (int n = 0; n < N; ++n) {
        float x = std::sin(twoPi * targetBin * n / N);
        buf[2 * n] = x; // real part
        buf[2 * n + 1] = 0.0f; // imag part
    }

    // Forward FFT
    p2t::smbFft(buf, N, -1);

    // Compute magnitudes
    std::vector<float> mags(N);
    for (int k = 0; k < N; ++k) {
        const float real = buf[2 * k];
        const float imag = buf[2 * k + 1];
        mags[k] = real * real + imag * imag;
    }

    // Check that target bin has the largest magnitude
    float targetMag = mags[targetBin];

    // All other bins should be ~0
    for (int k = 0; k < N; ++k) {
        if (k == targetBin) continue;

        EXPECT_NEAR(mags[k], 0.0f, 1e-3f)
            << "Bin " << k << " should be zero for perfect single-bin test";
    }

    // And the peak must be non-zero
    EXPECT_GT(targetMag, 100.0f)
        << "Target bin magnitude should be large";
}


