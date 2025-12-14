//
// Created by Moritz Seppelt on 14.11.25.
//

#ifndef PYTOTUNE_TEST_UTILS_H
#define PYTOTUNE_TEST_UTILS_H

#include <algorithm>
#include <vector>
#include <gtest/gtest.h>

namespace constants {
    const std::string TEST_DATA_DIR = "../tests/data";

    const std::string SIN_F440_I80_SR44100_AF1 = TEST_DATA_DIR + "/sin_f440_i80_sr44100_af1.wav";
    constexpr int SIN_FILE_NUM_SAMPLES = 44101;

    const std::string SIN_F440_I80_SR44100_AF3 = TEST_DATA_DIR + "/sin_f440_i80_sr44100_af3.wav";
    const std::string INVALID_FILE = TEST_DATA_DIR + "/invalid.wav";

    const std::string PIANO_F220_SR44100 = TEST_DATA_DIR + "/piano_f220_sr44100.wav";
    const std::string STRINGS_F440_SR44100 = TEST_DATA_DIR + "/strings_f440_sr44100.wav";
    const std::string VOICE_F400_SR4100 = TEST_DATA_DIR + "/voice_f440_sr44100.wav";
    const std::string SIN_RAISE_FSTART200_FEND1000_I80_SR44100_AF1 = TEST_DATA_DIR + "/sin-raise_fstart200_fend1000_i80_sr44100_af1.wav";
}


template<typename T>
::testing::AssertionResult SameMultisetImpl(
    const char *a_expr, const char *b_expr,
    const std::vector<T> &a,
    const std::vector<T> &b) {
    std::vector<T> sa = a;
    std::vector<T> sb = b;
    std::sort(sa.begin(), sa.end());
    std::sort(sb.begin(), sb.end());
constexpr const char *TEST_DATA_DIR = "../tests/data";


    if (sa == sb) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::AssertionFailure()
           << "Expected equality of these multisets:\n"
           << "  " << a_expr << " (sorted)\n"
           << "    Which is: " << ::testing::PrintToString(sa) << "\n"
           << "  " << b_expr << " (sorted)\n"
           << "    Which is: " << ::testing::PrintToString(sb) << "\n";
}


#define EXPECT_SAME_MULTISET(a, b) \
EXPECT_TRUE(SameMultisetImpl(#a, #b, a, b))

template<typename T>
::testing::AssertionResult NearVecImpl(
    const char *a_expr, const char *b_expr,
    const std::vector<T> &a,
    const std::vector<T> &b,
    T epsilon = T(1e-6)) {
    ::testing::AssertionResult failure = ::testing::AssertionFailure()
                                         << "Expected near equality of these vectors: (Epsilon: \n" << epsilon << ")\n"
                                         << "  " << a_expr << "\n"
                                         << "    Which is: " << ::testing::PrintToString(a) << "\n"
                                         << "  " << b_expr << "\n"
                                         << "    Which is: " << ::testing::PrintToString(b) << "\n";

    if (a.size() != b.size()) {
        return failure;
    }

    for (int i = 0; i < a.size(); i++) {
        const T diff = a[i] - b[i];
        if (diff > epsilon || diff < -epsilon)
            return failure << "Fails at i = " << i << " with: " << a[i] << " != " << b[i] << "\n";
    }

    return ::testing::AssertionSuccess();
}

#define EXPECT_NEAR_VEC(a, b) \
    EXPECT_TRUE(NearVecImpl(#a, #b, a, b));

#define EXPECT_NEAR_VEC_EPS(a, b, eps) \
    EXPECT_TRUE(NearVecImpl(#a, #b, a, b, eps));


#endif //PYTOTUNE_TEST_UTILS_H
