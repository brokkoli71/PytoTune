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

    const std::string SIN_FILE = TEST_DATA_DIR + "/sin_f440_i80_sr44100_af1.wav";
    constexpr int SIN_FILE_NUM_SAMPLES = 44101;

    const std::string SIN_AF3_FILE = TEST_DATA_DIR + "/sin_f440_i80_sr44100_af3.wav";
    const std::string INVALID_FILE = TEST_DATA_DIR + "/invalid.wav";

    const std::string PIANO_FILE = TEST_DATA_DIR + "/piano_f220_sr44100.wav";
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


#endif //PYTOTUNE_TEST_UTILS_H
