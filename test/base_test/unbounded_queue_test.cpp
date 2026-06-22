#include "unbounded_queue.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

class UnboundedQueueTest : public ::testing::Test {
protected:
    base::UnboundedQueue<int> queue_;
};

TEST_F(UnboundedQueueTest, NewQueueIsEmpty) {
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(queue_.Size(), 0u);
}

TEST_F(UnboundedQueueTest, EnqueueIncreasesSize) {
    queue_.Enqueue(42);
    EXPECT_FALSE(queue_.Empty());
    EXPECT_EQ(queue_.Size(), 1u);
}

TEST_F(UnboundedQueueTest, DequeueReturnsEnqueuedValue) {
    queue_.Enqueue(10);
    int val = 0;
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(queue_.Empty());
}

TEST_F(UnboundedQueueTest, DequeueFromEmptyReturnsFalse) {
    int val = 0;
    EXPECT_FALSE(queue_.Dequeue(&val));
    EXPECT_TRUE(queue_.Empty());
}

TEST_F(UnboundedQueueTest, FifoOrder) {
    queue_.Enqueue(1);
    queue_.Enqueue(2);
    queue_.Enqueue(3);
    EXPECT_EQ(queue_.Size(), 3u);

    int val = 0;
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(queue_.Empty());
}

TEST_F(UnboundedQueueTest, ClearEmptiesQueue) {
    queue_.Enqueue(1);
    queue_.Enqueue(2);
    queue_.Enqueue(3);
    ASSERT_FALSE(queue_.Empty());

    queue_.Clear();
    EXPECT_TRUE(queue_.Empty());
    EXPECT_EQ(queue_.Size(), 0u);
}

TEST_F(UnboundedQueueTest, CanEnqueueAfterClear) {
    queue_.Enqueue(1);
    queue_.Clear();
    queue_.Enqueue(42);

    int val = 0;
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 42);
}

TEST_F(UnboundedQueueTest, InterleavedEnqueueDequeue) {
    queue_.Enqueue(1);
    queue_.Enqueue(2);

    int val = 0;
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 1);

    queue_.Enqueue(3);
    queue_.Enqueue(4);

    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(queue_.Dequeue(&val));
    EXPECT_EQ(val, 4);
    EXPECT_TRUE(queue_.Empty());
}

TEST(UnboundedQueueNonTrivialTest, WorksWithString) {
    base::UnboundedQueue<std::string> q;
    q.Enqueue("hello");
    q.Enqueue("world");

    std::string val;
    EXPECT_TRUE(q.Dequeue(&val));
    EXPECT_EQ(val, "hello");
    EXPECT_TRUE(q.Dequeue(&val));
    EXPECT_EQ(val, "world");
}

TEST(UnboundedQueueNonTrivialTest, WorksWithVector) {
    base::UnboundedQueue<std::vector<int>> q;
    q.Enqueue({1, 2, 3});
    q.Enqueue({4, 5, 6});

    std::vector<int> val;
    EXPECT_TRUE(q.Dequeue(&val));
    EXPECT_EQ(val, (std::vector<int>{1, 2, 3}));
    EXPECT_TRUE(q.Dequeue(&val));
    EXPECT_EQ(val, (std::vector<int>{4, 5, 6}));
}

TEST(UnboundedQueueMoveTest, MoveOnlyTypeWorks) {
    base::UnboundedQueue<std::unique_ptr<int>> q;
    q.Enqueue(std::make_unique<int>(42));

    std::unique_ptr<int> val;
    EXPECT_TRUE(q.Dequeue(&val));
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, 42);
}

TEST(UnboundedQueueStressTest, ManyElements) {
    base::UnboundedQueue<int> q;
    const int N = 10000;

    for (int i = 0; i < N; ++i)
        q.Enqueue(i);
    EXPECT_EQ(q.Size(), static_cast<std::size_t>(N));

    for (int i = 0; i < N; ++i) {
        int val = -1;
        EXPECT_TRUE(q.Dequeue(&val));
        EXPECT_EQ(val, i);
    }
    EXPECT_TRUE(q.Empty());
}

TEST(UnboundedQueueDestructorTest, DestructorCleansUpNonEmpty) {
    {
        base::UnboundedQueue<int> q;
        q.Enqueue(1);
        q.Enqueue(2);
        q.Enqueue(3);
    }
    SUCCEED();
}

TEST(UnboundedQueueDestructorTest, DestructorOnEmptySucceeds) {
    { base::UnboundedQueue<int> q; }
    SUCCEED();
}
