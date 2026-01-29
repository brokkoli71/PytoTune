#include <gtest/gtest.h>

#include "pytotune/data-structures/windowing.h"

TEST(WindowingTest, TestWindowingConstructor) {
    const p2t::Windowing w1(100, 20);
    const p2t::Windowing w2(100, 0.8f);

    EXPECT_EQ(w1.windowSize, 100);
    EXPECT_EQ(w1.stride, 20);
    EXPECT_EQ(w2.stride, 20);
    EXPECT_EQ(w1.getOsamp(), 5);
};

TEST(WindowingTest, TestWindowedDataConstructor) {
    const p2t::Windowing w(100, 20);
    const std::vector<float> data = {1.f, 2.f, 3.f, 4.f};
    const p2t::WindowedData<float> wd(w, data);

    EXPECT_EQ(wd.windowing.windowSize, 100);
    EXPECT_EQ(wd.windowing.stride, 20);
    EXPECT_EQ(wd.data, data);
}

TEST(WindowingTest, TestFromLambdaCreation) {
    const p2t::Windowing w(100, 10);
    constexpr int dataSize = 5;
    constexpr float sampleRate = 2.f;
    const auto func = [](const float t) { return t * t; };

    const p2t::WindowedData<float> wd = p2t::WindowedData<float>::fromLambda(w, dataSize, sampleRate, func);
    EXPECT_EQ(wd.windowing.windowSize, 100);
    EXPECT_EQ(wd.windowing.stride, 10);
    EXPECT_EQ(wd.data, (std::vector<float>({0.f, 25.f, 100.f, 225.f, 400.f})));
}
