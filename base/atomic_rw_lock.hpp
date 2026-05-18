#ifndef BASE_ATOMIC_RW_LOCK_H
#define BASE_ATOMIC_RW_LOCK_H

#include <atomic>
#include <thread>
#include <cstdint>

namespace base {

/* 4 个标准动作 + 2 个标志量*/
class AtomicRWLock {
public:
    /* 首先定义规则 */
    static const int32_t RW_LOCK_FREE = 0;      // 房间没人
    static const int32_t WRITE_EXCLUSIVE = -1;  // VIP写者在里面
    static const uint32_t MAX_RETRY_TIMES = 5;  // 罚站五次就睡觉（混合自旋）

    AtomicRWLock() = default;
    explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}
    ~AtomicRWLock() = default;

    /* 解锁永远比加锁简单，不需要自选，不需要CAS，闭眼改屏幕就行 */
    void ReadUnlock();
    void WriteUnlock();

    void WriteLock();
    void ReadLock();

private:
    std::atomic<int32_t> lock_num_{0};              // 记录房间状态
    std::atomic<uint32_t> write_lock_wait_num_{0};  // 记录等待的VIP人数
    volatile bool write_first_{true};               // 是否先写者
};



inline void AtomicRWLock::ReadUnlock() {
    lock_num_.fetch_sub(1, std::memory_order_relaxed); // 简单减1
}

inline void AtomicRWLock::WriteUnlock() {
    lock_num_.fetch_add(1, std::memory_order_relaxed); // -1 + 1 瞬间归零
}

/* 写者加锁有四个标准动作 */
inline void AtomicRWLock::WriteLock() {
    // 动作 1：先在排号机上取个号（告诉所有读者：有VIP来了，都给我站住！）
    write_lock_wait_num_.fetch_add(1, std::memory_order_relaxed);

    // 动作 2 & 3：盯着主屏幕，只要不是 0，或者我没抢过别人，就在这里自旋！
    int32_t expected = RW_LOCK_FREE;
    uint32_t retry_times = 0;
    while (!lock_num_.compare_exchange_weak(expected, WRITE_EXCLUSIVE,
                                             std::memory_order_acq_rel,
                                             std::memory_order_relaxed)) {
        // 【致命细节】：如果CAS失败，expected 会被强行改成屏幕上的真实数字。
        // 比如有两个读者在里面，expected 就变成了 2。下次循环如果期望是 2 去改成 -1，系统就崩溃了！所以必须手动复位期望值！
        expected = RW_LOCK_FREE;

        // 动作 3的下半场：混合自旋逻辑。连续失败 5 次，主动让出 CPU
        if (++retry_times == MAX_RETRY_TIMES) {
            std::this_thread::yield();
            retry_times = 0;
        }
    }
    // 动作 4：历经千辛万苦，终于跳出了 while 循环，说明我成功把 0 变成了 -1！我进门了！
    // 既然我进门了，我就不在排队了，把排号机的数字减回去。
    write_lock_wait_num_.fetch_sub(1, std::memory_order_relaxed);
}

inline void AtomicRWLock::ReadLock() {
    uint32_t retry_times = 0;

    if (write_first_) {
        int32_t current_lock = lock_num_.load(std::memory_order_relaxed);
        // 最外层是一个 do-while 循环，负责 CAS 尝试
        do {
            // 【核心等待逻辑】：什么情况下读者绝对不能进？
            // 1. current_lock < RW_LOCK_FREE (说明VIP已经在里面了)
            // 2. write_lock_wait_num_.load() > 0 (说明门外有VIP在排队)
            while (current_lock < RW_LOCK_FREE || write_lock_wait_num_.load(std::memory_order_relaxed) > 0) {
                // 如果满足上述条件，就在门外罚站，顺便带上混合自旋
                if (++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                //  切回来之后再次读取门里面的状态
                current_lock = lock_num_.load(std::memory_order_relaxed);
            }
        } while (!lock_num_.compare_exchange_weak(current_lock, current_lock + 1,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed));
    } else {
        int32_t current_lock = lock_num_.load(std::memory_order_relaxed);
        do {
            while (current_lock < RW_LOCK_FREE) {
                if (++retry_times == MAX_RETRY_TIMES) {
                    std::this_thread::yield();
                    retry_times = 0;
                }
                current_lock = lock_num_.load(std::memory_order_relaxed);
            }
        } while (!lock_num_.compare_exchange_weak(current_lock, current_lock + 1,
                                                   std::memory_order_acq_rel,
                                                   std::memory_order_relaxed));
    }
}

} // namespace base

#endif
