//
// Created by Moritz Seppelt on 14.11.25.
//

#ifndef PYTOTUNE_TEST_UTILS_H
#define PYTOTUNE_TEST_UTILS_H

#include <algorithm>
#include <vector>


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


#endif //PYTOTUNE_TEST_UTILS_H
