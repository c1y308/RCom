#include "signal.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

// 测试信号触发时，所有已连接的槽函数都能按连接顺序收到参数。
TEST(SignalTest, EmitsArgumentsToAllConnectedSlots) {
    base::Signal<int, const std::string&> signal;
    std::vector<std::string> received;

    auto first = signal.connect([&](int value, const std::string& text) {
        received.emplace_back("first:" + std::to_string(value) + ":" + text);
    });
    auto second = signal.connect([&](int value, const std::string& text) {
        received.emplace_back("second:" + std::to_string(value * 2) + ":" + text);
    });

    ASSERT_TRUE(first.is_connected());
    ASSERT_TRUE(second.is_connected());

    signal(7, "ready");

    ASSERT_EQ(received.size(), 2u);
    EXPECT_EQ(received[0], "first:7:ready");
    EXPECT_EQ(received[1], "second:14:ready");
}

// 测试断开某个连接后，仅该连接停止接收信号，其他连接仍然有效。
TEST(SignalTest, DisconnectStopsOnlyThatConnection) {
    base::Signal<int> signal;
    int first_count = 0;
    int second_sum = 0;

    auto first = signal.connect([&](int) { ++first_count; });
    auto second = signal.connect([&](int value) { second_sum += value; });

    signal(3);
    EXPECT_EQ(first_count, 1);
    EXPECT_EQ(second_sum, 3);

    EXPECT_TRUE(first.disconnect());
    EXPECT_FALSE(first.is_connected());
    EXPECT_TRUE(second.is_connected());

    signal(5);
    EXPECT_EQ(first_count, 1);
    EXPECT_EQ(second_sum, 8);
}

// 测试同一个连接重复断开时，第一次成功，后续断开返回 false。
TEST(SignalTest, DisconnectReturnsFalseWhenConnectionIsAlreadyRemoved) {
    base::Signal<int> signal;
    int calls = 0;

    auto connection = signal.connect([&](int) { ++calls; });

    EXPECT_TRUE(connection.disconnect());
    EXPECT_FALSE(connection.disconnect());

    signal(1);
    EXPECT_EQ(calls, 0);
}

// 测试 disconnect_all_slots 会断开所有槽函数，并阻止后续信号触发它们。
TEST(SignalTest, DisconnectAllSlotsStopsFutureEmits) {
    base::Signal<int> signal;
    int sum = 0;

    auto first = signal.connect([&](int value) { sum += value; });
    auto second = signal.connect([&](int value) { sum += value * 10; });

    signal(2);
    EXPECT_EQ(sum, 22);

    signal.disconnect_all_slots();
    EXPECT_FALSE(first.is_connected());
    EXPECT_FALSE(second.is_connected());

    signal(3);
    EXPECT_EQ(sum, 22);
}

// 测试无参数 Signal<> 可以正常连接、触发和断开。
TEST(SignalTest, SupportsSignalsWithoutArguments) {
    base::Signal<> signal;
    int calls = 0;

    auto connection = signal.connect([&] { ++calls; });

    signal();
    EXPECT_EQ(calls, 1);

    EXPECT_TRUE(connection.disconnect());
    signal();
    EXPECT_EQ(calls, 1);
}

// 测试信号触发过程中断开尚未执行的槽函数时，该槽函数不会被调用。
TEST(SignalTest, SlotDisconnectedDuringEmitIsNotCalled) {
    base::Signal<> signal;
    int first_calls = 0;
    int second_calls = 0;

    base::Connection<> second;
    auto first = signal.connect([&] {
        ++first_calls;
        EXPECT_TRUE(second.disconnect());
    });
    second = signal.connect([&] { ++second_calls; });

    ASSERT_TRUE(first.is_connected());
    ASSERT_TRUE(second.is_connected());

    signal();

    EXPECT_EQ(first_calls, 1);
    EXPECT_EQ(second_calls, 0);
    EXPECT_TRUE(first.is_connected());
    EXPECT_FALSE(second.is_connected());
}

// 测试槽函数在执行过程中断开自身后，后续信号不会再触发它。
TEST(SignalTest, SelfDisconnectDuringEmitStopsLaterEmits) {
    base::Signal<> signal;
    int calls = 0;

    base::Connection<> connection;
    connection = signal.connect([&] {
        ++calls;
        EXPECT_TRUE(connection.disconnect());
    });

    signal();
    signal();

    EXPECT_EQ(calls, 1);
    EXPECT_FALSE(connection.is_connected());
}
