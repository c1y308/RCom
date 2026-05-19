#ifndef BASE_WAIT_STRATEGY_H
#define BASE_WAIT_STRATEGY_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>

namespace base{

class WaitStrategy{
public:
    virtual ~WaitStrategy() {}

    virtual void notifyOne() {}
    virtual void breakAllWait() {}
    virtual bool emptyWait() = 0;
}; 

/* 1.自旋等待策略 */
class BusySpinWaitStrategy : public WaitStrategy {
 public:
  BusySpinWaitStrategy() {}
  bool emptyWait() override { return true; }
};

/* 2.阻塞等待策略 */
class BlockWaitStrategy : public WaitStrategy{
public:
    BlockWaitStrategy() {}
    ~BlockWaitStrategy() {}

    void notifyOne() override {
        std::lock_guard<std::mutex> lock(mutex_);
        ++notify_count_;
        cv_.notify_one();
    }

    void breakAllWait() override {
        std::lock_guard<std::mutex> lock(mutex_);
        break_all_wait_ = true;
        cv_.notify_all();
    }

    bool emptyWait() override {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] {
            return notify_count_ > 0 || break_all_wait_;
        });

        if(break_all_wait_)
            return false;

        --notify_count_;
        return true;
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    uint64_t notify_count_ = 0;
    bool break_all_wait_ = false;
};

/* 3.超时阻塞等待策略 */
class TimeoutBlockWaitStrategy : public WaitStrategy {
 public:
  TimeoutBlockWaitStrategy() {}
  explicit TimeoutBlockWaitStrategy(uint64_t timeout)
      : time_out_(std::chrono::milliseconds(timeout)) {}

  void notifyOne() override {
    std::lock_guard<std::mutex> lock(mutex_);
    ++notify_count_;
    cv_.notify_one();
  }

  bool emptyWait() override {
    /* 线程阻塞 如果超时了则返回false，没超时则返回true*/
    std::unique_lock<std::mutex> lock(mutex_);
    if (!cv_.wait_for(lock, time_out_, [this] {
          return notify_count_ > 0 || break_all_wait_;
        })) {
      return false;
    }
    if (break_all_wait_) {
      return false;
    }
    --notify_count_;
    return true;
  }

  void breakAllWait() override {
    std::lock_guard<std::mutex> lock(mutex_);
    break_all_wait_ = true;
    cv_.notify_all();
  }

  void setTimeout(uint64_t timeout) {
    time_out_ = std::chrono::milliseconds(timeout);
  }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  std::chrono::milliseconds time_out_{10000};
  uint64_t notify_count_ = 0;
  bool break_all_wait_ = false;
};

/* 4.睡眠等待策略 */
class SleepWaitStrategy : public WaitStrategy{
public:
    SleepWaitStrategy() {}
    explicit SleepWaitStrategy(uint64_t timeout_ms) : timeout_ms_(timeout_ms) {}
    ~SleepWaitStrategy() {}

    bool emptyWait() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms_));
        return true;
    }

    void setSleepMilliSeconds(uint64_t timeout_ms) {
        timeout_ms_ = timeout_ms;
    }

private:
    uint64_t timeout_ms_ = 10000;
};

/* 5.调度等待策略 */
class ScheduleWaitStrategy : public WaitStrategy{
public:
    ScheduleWaitStrategy() {}
    ~ScheduleWaitStrategy() {}

    bool emptyWait() override {
        std::this_thread::yield();
        return true;
    }
};

} // namespace base

#endif
