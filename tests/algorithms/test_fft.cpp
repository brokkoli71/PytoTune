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

// Test A: Real sine -> two symmetric bins (k and N-k)
TEST(SmbFftTest, RealSine_TwoSymmetricBins) {
    const int N = 1024;
    const float fs = 48000.0f;
    const int k = 10; // target bin
    const float freq = k * fs / N; // exact bin freq
    const float twoPi = 2.0f * M_PI;

    std::vector<float> buf(2 * N, 0.0f);
    for (int n = 0; n < N; ++n) {
        float x = std::sin(twoPi * freq * n / fs); // real sine
        buf[2 * n] = x; // real
        buf[2 * n + 1] = 0.0f; // imag = 0
    }

    p2t::smbFft(buf, N, -1);

    auto mag = [&](int idx) {
        float r = buf[2 * idx];
        float i = buf[2 * idx + 1];
        return std::sqrt(r * r + i * i);
    };

    const int mirror = (N - k) % N;
    const float eps = 1e-3f;

    // Check symmetric bins are large
    EXPECT_GT(mag(k), 1e-3f);
    EXPECT_GT(mag(mirror), 1e-3f);

    // All other bins near zero
    for (int b = 0; b < N; ++b) {
        if (b == k || b == mirror) continue;
        EXPECT_NEAR(mag(b), 0.0f, eps) << "Unexpected energy in bin " << b;
    }
}

// Test B: Complex exponential -> single bin k non-zero
TEST(SmbFftTest, ComplexExponential_SingleBin) {
    const int N = 1024;
    const float fs = 48000.0f;
    const int k = 10;
    const float freq = k * fs / N;
    const float twoPi = 2.0f * M_PI;

    std::vector<float> buf(2 * N, 0.0f);
    // Fill with complex exponential e^{j 2π k n / N} = cos(...) + j sin(...)
    for (int n = 0; n < N; ++n) {
        float angle = twoPi * freq * n / fs; // == 2π * k * n / N
        buf[2 * n] = std::cos(angle); // real part
        buf[2 * n + 1] = std::sin(angle); // imag part
    }

    p2t::smbFft(buf, N, -1);

    auto mag = [&](int idx) {
        float r = buf[2 * idx];
        float i = buf[2 * idx + 1];
        return std::sqrt(r * r + i * i);
    };

    const float eps = 1e-3f;

    // Target bin should be large
    EXPECT_GT(mag(k), 1e-3f);

    // All other bins ~0
    for (int b = 0; b < N; ++b) {
        if (b == k) continue;
        EXPECT_NEAR(mag(b), 0.0f, eps) << "Unexpected energy in bin " << b;
    }
}



