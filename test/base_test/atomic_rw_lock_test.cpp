#include "atomic_rw_lock.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace {

template <typename Predicate>
bool WaitUntil(Predicate predicate,
               std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while(std::chrono::steady_clock::now() < deadline){
        if(predicate()){
            return true;
        }
        std::this_thread::yield();
    }
    return predicate();
}

} // namespace

TEST(AtomicRWLockTest, ReadLockAllowsMultipleReaders) {
    base::AtomicRWLock lock;
    lock.read__lock();

    std::atomic<bool> reader_acquired{false};
    std::atomic<bool> release_reader{false};

    std::thread reader([&] {
        lock.read__lock();
        reader_acquired.store(true, std::memory_order_release);
        while(!release_reader.load(std::memory_order_acquire)){
            std::this_thread::yield();
        }
        lock.read__unlock();
    });

    EXPECT_TRUE(WaitUntil([&] {
        return reader_acquired.load(std::memory_order_acquire);
    }));

    release_reader.store(true, std::memory_order_release);
    reader.join();
    lock.read__unlock();
}

TEST(AtomicRWLockTest, WriteLockWaitsForReaders) {
    base::AtomicRWLock lock;
    lock.read__lock();

    std::atomic<bool> writer_acquired{false};
    std::atomic<bool> release_writer{false};

    std::thread writer([&] {
        lock.write__lock();
        writer_acquired.store(true, std::memory_order_release);
        while(!release_writer.load(std::memory_order_acquire)){
            std::this_thread::yield();
        }
        lock.write__unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(writer_acquired.load(std::memory_order_acquire));

    lock.read__unlock();
    EXPECT_TRUE(WaitUntil([&] {
        return writer_acquired.load(std::memory_order_acquire);
    }));

    release_writer.store(true, std::memory_order_release);
    writer.join();
}

TEST(AtomicRWLockTest, ReadLockWaitsForWriter) {
    base::AtomicRWLock lock;
    lock.write__lock();

    std::atomic<bool> reader_acquired{false};

    std::thread reader([&] {
        lock.read__lock();
        reader_acquired.store(true, std::memory_order_release);
        lock.read__unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(reader_acquired.load(std::memory_order_acquire));

    lock.write__unlock();
    EXPECT_TRUE(WaitUntil([&] {
        return reader_acquired.load(std::memory_order_acquire);
    }));
    reader.join();
}

TEST(AtomicRWLockTest, WriteFirstBlocksNewReadersBehindWaitingWriter) {
    base::AtomicRWLock lock(true);
    lock.read__lock();

    std::atomic<bool> writer_started{false};
    std::atomic<bool> writer_acquired{false};
    std::atomic<bool> release_writer{false};

    std::thread writer([&] {
        writer_started.store(true, std::memory_order_release);
        lock.write__lock();
        writer_acquired.store(true, std::memory_order_release);
        while(!release_writer.load(std::memory_order_acquire)){
            std::this_thread::yield();
        }
        lock.write__unlock();
    });

    ASSERT_TRUE(WaitUntil([&] {
        return writer_started.load(std::memory_order_acquire);
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::atomic<bool> reader_acquired{false};
    std::thread reader([&] {
        lock.read__lock();
        reader_acquired.store(true, std::memory_order_release);
        lock.read__unlock();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(reader_acquired.load(std::memory_order_acquire));

    lock.read__unlock();
    EXPECT_TRUE(WaitUntil([&] {
        return writer_acquired.load(std::memory_order_acquire);
    }));
    EXPECT_FALSE(reader_acquired.load(std::memory_order_acquire));

    release_writer.store(true, std::memory_order_release);
    writer.join();

    EXPECT_TRUE(WaitUntil([&] {
        return reader_acquired.load(std::memory_order_acquire);
    }));
    reader.join();
}

TEST(AtomicRWLockTest, ReaderFirstAllowsNewReadersWhileWriterWaits) {
    base::AtomicRWLock lock(false);
    lock.read__lock();

    std::atomic<bool> writer_started{false};
    std::atomic<bool> writer_acquired{false};
    std::atomic<bool> release_writer{false};

    std::thread writer([&] {
        writer_started.store(true, std::memory_order_release);
        lock.write__lock();
        writer_acquired.store(true, std::memory_order_release);
        while(!release_writer.load(std::memory_order_acquire)){
            std::this_thread::yield();
        }
        lock.write__unlock();
    });

    ASSERT_TRUE(WaitUntil([&] {
        return writer_started.load(std::memory_order_acquire);
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    std::atomic<bool> reader_acquired{false};
    std::thread reader([&] {
        lock.read__lock();
        reader_acquired.store(true, std::memory_order_release);
        lock.read__unlock();
    });

    EXPECT_TRUE(WaitUntil([&] {
        return reader_acquired.load(std::memory_order_acquire);
    }));
    EXPECT_FALSE(writer_acquired.load(std::memory_order_acquire));

    reader.join();
    lock.read__unlock();

    EXPECT_TRUE(WaitUntil([&] {
        return writer_acquired.load(std::memory_order_acquire);
    }));
    release_writer.store(true, std::memory_order_release);
    writer.join();
}

TEST(AtomicRWLockStressTest, ReadersAndWritersDoNotOverlapIncorrectly) {
    constexpr int kReaderCount = 4;
    constexpr int kWriterCount = 2;
    constexpr int kIterations = 1000;

    base::AtomicRWLock lock;
    std::atomic<int> active_readers{0};
    std::atomic<int> active_writers{0};
    std::atomic<int> violations{0};
    std::atomic<int> writes{0};

    std::vector<std::thread> threads;
    for(int i = 0; i < kReaderCount; ++i){
        threads.emplace_back([&] {
            for(int iteration = 0; iteration < kIterations; ++iteration){
                lock.read__lock();
                if(active_writers.load() != 0){
                    violations.fetch_add(1);
                }
                active_readers.fetch_add(1);
                std::this_thread::yield();
                if(active_writers.load() != 0){
                    violations.fetch_add(1);
                }
                active_readers.fetch_sub(1);
                lock.read__unlock();
            }
        });
    }

    for(int i = 0; i < kWriterCount; ++i){
        threads.emplace_back([&] {
            for(int iteration = 0; iteration < kIterations; ++iteration){
                lock.write__lock();
                if(active_writers.fetch_add(1) != 0 || active_readers.load() != 0){
                    violations.fetch_add(1);
                }
                std::this_thread::yield();
                active_writers.fetch_sub(1);
                writes.fetch_add(1);
                lock.write__unlock();
            }
        });
    }

    for(auto &thread : threads){
        thread.join();
    }

    EXPECT_EQ(violations.load(), 0);
    EXPECT_EQ(writes.load(), kWriterCount * kIterations);
}
