#ifndef BASE_ATOMIC_RW_LOCK_H
#define BASE_ATOMIC_RW_LOCK_H

#include <atomic>
#include <thread>
#include <cstdint>

namespace base {

/* 4 个标准动作 + 2 个标志量*/
class AtomicRWLock {
public:
    /* 定义规则 */
    static const int32_t  RW_LOCK_FREE = 0;      // 房间没人
    static const int32_t  WRITE_EXCLUSIVE = -1;  // 写者在里面（VIP态）
    static const uint32_t MAX_RETRY_TIMES = 5;   // 罚站五次就睡觉（混合自旋）

    AtomicRWLock() = default;
    explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}
    ~AtomicRWLock() = default;

    /* 解锁永远比加锁简单，不需要自旋，不需要CAS，闭眼改房间状态就行 */
    void ReadUnlock();
    void WriteUnlock();

    void WriteLock();
    void ReadLock();

private:
    std::atomic<int32_t> lock_num_{0};              // 房间状态(记录读者人数，写者占据为 -1)
    std::atomic<uint32_t> write_lock_wait_num_{0};  // 等待的写者数
    bool write_first_{true};                           // 是否先写者
};



inline void AtomicRWLock::ReadUnlock() {
    lock_num_.fetch_sub(1, std::memory_order_release);
}

inline void AtomicRWLock::WriteUnlock() {
    lock_num_.fetch_add(1, std::memory_order_release); // 写者占据是 -1
}

/* 写者加锁 */
inline void AtomicRWLock::WriteLock() {
    // 先在写者排号机上取个号
    write_lock_wait_num_.fetch_add(1, std::memory_order_relaxed);

    // 只有房间为空 (0) 时，才能通过CAS进行抢进入资格！
    int32_t expected = RW_LOCK_FREE;
    uint32_t retry_times = 0;
    // 同一时刻只有一个写者抢到进入资格(成功则将房间状态置为 -1)
    while (!lock_num_.compare_exchange_weak(expected, WRITE_EXCLUSIVE,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
        // 如果CAS失败，expected 会被改成房间状态，手动复位期望值
        expected = RW_LOCK_FREE;

        // 混合自旋：连续失败 5 次，主动让出 CPU
        if (++retry_times == MAX_RETRY_TIMES) {
            std::this_thread::yield();
            retry_times = 0;
        }
    }
    // 跳出了 while 循环进门成功，把排号机的数字减回去。
    write_lock_wait_num_.fetch_sub(1, std::memory_order_relaxed);
}

inline void AtomicRWLock::ReadLock() {
    uint32_t retry_times = 0;

    if (write_first_) {
        //  先读取本轮当前房间状态
        int32_t current_lock = lock_num_.load(std::memory_order_relaxed);
        // 最外层是一个 do-while 循环，先检查能不能进行尝试再尝试
        do {
            // 【核心等待逻辑】：什么情况下读者不能进？
            // 1. current_lock < RW_LOCK_FREE (写者（VIP）在里面)
            // 2. write_lock_wait_num_.load() > 0 (门外有VIP在排队)
            while (current_lock < RW_LOCK_FREE || write_lock_wait_num_.load(std::memory_order_relaxed) > 0) {
                // 就在门外罚站，顺便带上混合自旋
                if (++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                //  切回来之后再次读取门里面的状态
                current_lock = lock_num_.load(std::memory_order_relaxed);
            }
        } // 同一时刻只有一个读者抢到进入资格
        while (!lock_num_.compare_exchange_weak(current_lock, current_lock + 1,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed));
    } else {
        //  先读取本轮当前房间状态
        int32_t current_lock = lock_num_.load(std::memory_order_relaxed);
        // 最外层是一个 do-while 循环，先检查能不能进行尝试再尝试
        do {
            while (current_lock < RW_LOCK_FREE) {
                if (++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                current_lock = lock_num_.load(std::memory_order_relaxed);
            }
        } // 同一时刻只有一个读者抢到进入资格
        while (!lock_num_.compare_exchange_weak(current_lock, current_lock + 1,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed));
    }
}

} // namespace base

#endif
