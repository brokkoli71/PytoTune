//
// Created by Moritz Seppelt on 14.11.25.
//

#ifndef PYTOTUNE_TEST_UTILS_H
#define PYTOTUNE_TEST_UTILS_H

#include <algorithm>
#include <vector>
#include <gtest/gtest.h>


constexpr const char *TEST_DATA_DIR = "../tests/data";

template<typename T>
::testing::AssertionResult SameMultisetImpl(
    const char *a_expr, const char *b_expr,
    const std::vector<T> &a,
    const std::vector<T> &b) {
    std::vector<T> sa = a;
    std::vector<T> sb = b;
    std::sort(sa.begin(), sa.end());
    std::sort(sb.begin(), sb.end());

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
            return failure;
    }

    return ::testing::AssertionSuccess();
}

#define EXPECT_NEAR_VEC(a, b) \
    EXPECT_TRUE(NearVecImpl(#a, #b, a, b));


#endif //PYTOTUNE_TEST_UTILS_H
