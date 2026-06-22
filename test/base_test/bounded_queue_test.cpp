#include "bounded_queue.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

TEST(BoundedQueueInitTest, RejectsInvalidInit) {
    base::BoundedQueue<int> queue;

    EXPECT_FALSE(queue.Init(0));
    EXPECT_FALSE(queue.Init(4, nullptr));
    EXPECT_TRUE(queue.Init(2, new base::ScheduleWaitStrategy()));
    EXPECT_FALSE(queue.Init(2, new base::ScheduleWaitStrategy()));
}

TEST(BoundedQueueTest, NewQueueIsEmpty) {
    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(4, new base::ScheduleWaitStrategy()));

    EXPECT_TRUE(queue.Empty());
    EXPECT_EQ(queue.Size(), 0u);
    EXPECT_EQ(queue.Head(), 0u);
    EXPECT_EQ(queue.Tail(), 0u);
    EXPECT_EQ(queue.Commit(), 0u);
}

TEST(BoundedQueueTest, FifoOrderAndCapacity) {
    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(2, new base::ScheduleWaitStrategy()));

    EXPECT_TRUE(queue.Enqueue(1));
    EXPECT_TRUE(queue.Enqueue(2));
    EXPECT_FALSE(queue.Enqueue(3));
    EXPECT_EQ(queue.Size(), 2u);

    int value = 0;
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(queue.Enqueue(3));

    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 2);
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 3);
    EXPECT_FALSE(queue.Dequeue(&value));
    EXPECT_TRUE(queue.Empty());
}

TEST(BoundedQueueTest, CapacityOneWorks) {
    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(1, new base::ScheduleWaitStrategy()));

    EXPECT_TRUE(queue.Enqueue(1));
    EXPECT_FALSE(queue.Enqueue(2));

    int value = 0;
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 1);

    EXPECT_TRUE(queue.Enqueue(2));
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 2);
}

TEST(BoundedQueueMoveTest, MoveOnlyTypeWorks) {
    base::BoundedQueue<std::unique_ptr<int>> queue;
    ASSERT_TRUE(queue.Init(2, new base::ScheduleWaitStrategy()));

    EXPECT_TRUE(queue.Enqueue(std::make_unique<int>(42)));

    std::unique_ptr<int> value;
    EXPECT_TRUE(queue.Dequeue(&value));
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
}

TEST(BoundedQueueWaitTest, WaitDequeueWakesAfterEnqueue) {
    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(1, new base::TimeoutBlockWaitStrategy(500)));

    std::atomic<bool> started{false};
    bool ok = false;
    int value = 0;

    std::thread consumer([&] {
        started.store(true, std::memory_order_release);
        ok = queue.waitDequeue(&value);
    });

    while(!started.load(std::memory_order_acquire)){
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_TRUE(queue.Enqueue(7));
    consumer.join();

    EXPECT_TRUE(ok);
    EXPECT_EQ(value, 7);
}

TEST(BoundedQueueWaitTest, WaitEnqueueWakesAfterDequeue) {
    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(1, new base::TimeoutBlockWaitStrategy(500)));
    ASSERT_TRUE(queue.Enqueue(1));

    std::atomic<bool> started{false};
    bool ok = false;

    std::thread producer([&] {
        started.store(true, std::memory_order_release);
        ok = queue.waitEnqueue(2);
    });

    while(!started.load(std::memory_order_acquire)){
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    int value = 0;
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 1);
    producer.join();

    EXPECT_TRUE(ok);
    EXPECT_TRUE(queue.Dequeue(&value));
    EXPECT_EQ(value, 2);
}

TEST(BoundedQueueStressTest, MultipleProducersMultipleConsumers) {
    constexpr int kProducerCount = 4;
    constexpr int kConsumerCount = 4;
    constexpr int kItemsPerProducer = 1000;
    constexpr int kTotalItems = kProducerCount * kItemsPerProducer;

    base::BoundedQueue<int> queue;
    ASSERT_TRUE(queue.Init(64, new base::ScheduleWaitStrategy()));

    std::atomic<int> consumed{0};
    std::atomic<int> errors{0};
    std::vector<std::atomic<int>> seen(kTotalItems);
    for(auto &count : seen){
        count.store(0, std::memory_order_relaxed);
    }

    std::vector<std::thread> producers;
    for(int producer = 0; producer < kProducerCount; ++producer){
        producers.emplace_back([&, producer] {
            for(int item = 0; item < kItemsPerProducer; ++item){
                int value = producer * kItemsPerProducer + item;
                while(!queue.Enqueue(value)){
                    std::this_thread::yield();
                }
            }
        });
    }

    std::vector<std::thread> consumers;
    for(int consumer = 0; consumer < kConsumerCount; ++consumer){
        consumers.emplace_back([&] {
            while(consumed.load(std::memory_order_acquire) < kTotalItems){
                int value = -1;
                if(queue.Dequeue(&value)){
                    if(value < 0 || value >= kTotalItems){
                        errors.fetch_add(1, std::memory_order_relaxed);
                    }else{
                        seen[value].fetch_add(1, std::memory_order_relaxed);
                    }
                    consumed.fetch_add(1, std::memory_order_release);
                }else{
                    std::this_thread::yield();
                }
            }
        });
    }

    for(auto &producer : producers){
        producer.join();
    }
    for(auto &consumer : consumers){
        consumer.join();
    }

    EXPECT_EQ(consumed.load(), kTotalItems);
    EXPECT_EQ(errors.load(), 0);
    for(const auto &count : seen){
        EXPECT_EQ(count.load(), 1);
    }
    EXPECT_TRUE(queue.Empty());
}
