#include "duration.hpp"
#include "rate.hpp"
#include "time.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <thread>

TEST(DurationTest, ConvertsBetweenSecondsAndNanoseconds) {
    const timer::Duration from_nanoseconds(std::int64_t{1500000000});
    const timer::Duration from_seconds(1.5);
    const timer::Duration from_parts(1U, 500000000U);

    EXPECT_DOUBLE_EQ(from_nanoseconds.to_seconds(), 1.5);
    EXPECT_EQ(from_seconds.to_nanoseconds(), 1500000000);
    EXPECT_EQ(from_parts.to_nanoseconds(), 1500000000);
    EXPECT_FALSE(from_nanoseconds.is_zero());
    EXPECT_TRUE(timer::Duration(std::int64_t{0}).is_zero());
}

TEST(DurationTest, SupportsArithmeticAndComparison) {
    const timer::Duration one_second(1.0);
    const timer::Duration half_second(0.5);

    EXPECT_EQ((one_second + half_second).to_nanoseconds(), 1500000000);
    EXPECT_EQ((one_second - half_second).to_nanoseconds(), 500000000);
    EXPECT_EQ((-half_second).to_nanoseconds(), -500000000);
    EXPECT_EQ((half_second * 3.0).to_nanoseconds(), 1500000000);
    EXPECT_GT(one_second, half_second);
    EXPECT_LE(half_second, one_second);
}

TEST(TimeTest, ConvertsAndComparesTimeValues) {
    const timer::Time time(2U, 500U);

    EXPECT_EQ(time.to_nanoseconds(), 2000000500U);
    EXPECT_EQ(time.to_microseconds(), 2000000U);
    EXPECT_DOUBLE_EQ(time.to_seconds(), 2.0000005);
    EXPECT_FALSE(time.is_zero());
    EXPECT_TRUE(timer::Time::MIN.is_zero());
    EXPECT_GT(timer::Time::MAX, time);
}

TEST(TimeTest, SupportsDurationArithmetic) {
    const timer::Time start(2U, 0U);
    const timer::Duration delta(0U, 250U);

    const timer::Time end = start + delta;

    EXPECT_EQ(end.to_nanoseconds(), 2000000250U);
    EXPECT_EQ((end - start).to_nanoseconds(), 250);
    EXPECT_EQ((end - delta).to_nanoseconds(), start.to_nanoseconds());
}

TEST(TimeTest, NowMovesForward) {
    const auto first = timer::Time::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const auto second = timer::Time::now();

    EXPECT_GE(second, first);
}

TEST(RateTest, StoresExpectedCycleTimeFromFrequency) {
    const timer::Rate rate(10.0);

    EXPECT_EQ(rate.expected_cycleTime().to_nanoseconds(), 100000000);
    EXPECT_TRUE(rate.cycle_time().is_zero());
}

TEST(RateTest, StoresExpectedCycleTimeFromDuration) {
    const timer::Duration duration(std::int64_t{2000000});
    const timer::Rate rate(duration);

    EXPECT_EQ(rate.expected_cycleTime(), duration);
}
