#ifndef __BASE_RW_LOCK_GUARD_H__
#define __BASE_RW_LOCK_GUARD_H__

#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>



/**
 * @file rw_lock_guard.hpp
 * @author c1y308
 * @brief 提供读写锁的 RAII 守卫，负责在作用域进入和退出时自动加解锁。
 */
namespace base {

template <typename RWLock>
class ReadLockGuard {
public:
    explicit ReadLockGuard(RWLock& lock) : rw_lock_(lock) { rw_lock_.read__lock(); }

    ~ReadLockGuard() { rw_lock_.read__unlock(); }

private:
    ReadLockGuard(const ReadLockGuard& other) = delete;
    ReadLockGuard& operator=(const ReadLockGuard& other) = delete;
    RWLock& rw_lock_;
};


template <typename RWLock>
class WriteLockGuard {
public:
    explicit WriteLockGuard(RWLock& lock) : rw_lock_(lock) {
        rw_lock_.write__lock();
    }

    ~WriteLockGuard() { rw_lock_.write__unlock(); }

private:
    /* 禁止 拷贝构造 和 拷贝赋值 */
    WriteLockGuard(const WriteLockGuard& other) = delete;
    WriteLockGuard& operator=(const WriteLockGuard& other) = delete;
    RWLock& rw_lock_;
};

}


#endif
