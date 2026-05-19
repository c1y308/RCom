#ifndef BASE_BOUNDED_QUEUE_H
#define BASE_BOUNDED_QUEUE_H
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <utility>
#include "wait_strategy.hpp"

namespace base{

/* 以数组连续形式存储 */
template<typename T>
class BoundedQueue{
public:
    using value_type = T;
    using size_type = uint64_t;

    BoundedQueue() {}
    BoundedQueue& operator=(const BoundedQueue &other) = delete;
    BoundedQueue(const BoundedQueue &other) = delete;
    ~BoundedQueue();

    /* 默认线程阻塞策略为睡眠策略 */
    bool Init(uint64_t size);
    bool Init(uint64_t size, WaitStrategy *strategy);

    bool Enqueue(const T &element);
    bool Enqueue(T &&element);

    bool waitEnqueue(const T &element);
    bool waitEnqueue(T &&element);

    bool Dequeue(T *element);
    bool waitDequeue(T *element);

    uint64_t Size();
    bool Empty();

    void setWaitStrategy(WaitStrategy *strategy);
    void breakAllWait();

    uint64_t Head();
    uint64_t Tail();
    uint64_t Commit();
private:
    struct Cell{
        std::atomic<uint64_t> sequence;
        T data;

        Cell() : sequence(0), data() {}
    };

    template<typename U>
    bool EnqueueImpl(U &&element);

    uint64_t getIndex(uint64_t num) const;
    uint64_t EmptySequence(uint64_t pos) const;
    uint64_t FullSequence(uint64_t pos) const;

    static constexpr std::size_t CACHELINE_SIZE = 64;

    alignas(CACHELINE_SIZE) std::atomic<uint64_t> enqueue_pos_ = {0};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> dequeue_pos_ = {0};

    bool init_ = false;
    Cell *buffer_ = nullptr;
    uint64_t capacity_ = 0;

    std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;  // 指向等待策略类的实例
    std::atomic<bool> break_all_wait_ = {false};
};


template <typename T>
BoundedQueue<T>::~BoundedQueue() {
    if (wait_strategy_) {
        breakAllWait();
    }
    delete[] buffer_;
}

/* 默认线程阻塞策略为睡眠策略 */
template<typename T>
inline bool BoundedQueue<T>::Init(uint64_t size){
    auto strategy = std::unique_ptr<WaitStrategy>(new (std::nothrow) SleepWaitStrategy());
    if(strategy == nullptr)
        return false;
    return Init(size, strategy.release());
}

/* 分配内存/设置队列的等待策略 */
template<typename T>
inline bool BoundedQueue<T>::Init(uint64_t size, WaitStrategy *strategy){
    std::unique_ptr<WaitStrategy> strategy_guard(strategy);
    if(size == 0 || strategy_guard == nullptr || init_)
        return false;
    /* 分配内存 */
    Cell *buffer = nullptr;
    try{
        buffer = new (std::nothrow) Cell[size];
    }catch(...){
        return false;
    }
    if(buffer == nullptr)
        return false;

    /* 初始化序列号（每个槽位设置为空状态） */
    for(uint64_t i = 0; i < size; i++){
        buffer[i].sequence.store(EmptySequence(i), std::memory_order_relaxed);
    }

    buffer_ = buffer;
    capacity_ = size;
    enqueue_pos_.store(0, std::memory_order_relaxed);
    dequeue_pos_.store(0, std::memory_order_relaxed);
    wait_strategy_ = std::move(strategy_guard);
    break_all_wait_.store(false, std::memory_order_release);
    init_ = true;
    return true;
}


template<typename T>
bool BoundedQueue<T>::Enqueue(const T &element){
    return EnqueueImpl(element);
}

template<typename T>
bool BoundedQueue<T>::Enqueue(T &&element){
    return EnqueueImpl(std::move(element));
}

template <typename T>
bool BoundedQueue<T>::waitEnqueue(const T &element) {
    while(!break_all_wait_.load(std::memory_order_acquire)){
        if(Enqueue(element))
            return true;

        if(wait_strategy_ == nullptr)
            return false;

        if(wait_strategy_->emptyWait())
            continue;

        // wait timeout or breakAllWait
        break;
    }

    return false;
}

template<typename T>
bool BoundedQueue<T>::waitEnqueue(T &&element){
    while(!break_all_wait_.load(std::memory_order_acquire)){
        if(Enqueue(std::move(element)))
            return true;

        if(wait_strategy_ == nullptr)
            return false;

        if(wait_strategy_->emptyWait())
            continue;

        // wait timeout or breakAllWait
        break;
    }

    return false;
}



template<typename T>
bool BoundedQueue<T>::Dequeue(T *element){
    if(!init_ || element == nullptr)
        return false;

    Cell *cell = nullptr;
    uint64_t pos = dequeue_pos_.load(std::memory_order_relaxed);

    while(true){
        cell = &buffer_[getIndex(pos)];
        uint64_t sequence = cell->sequence.load(std::memory_order_acquire);
        uint64_t expected = FullSequence(pos);

        if(sequence == expected){
            if(dequeue_pos_.compare_exchange_weak(pos, pos + 1,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed)){
                break;
            }
        }else if(sequence < expected){
            return false;
        }else{
            pos = dequeue_pos_.load(std::memory_order_relaxed);
        }
    }

    *element = std::move_if_noexcept(cell->data);
    cell->sequence.store(EmptySequence(pos + capacity_), std::memory_order_release);
    if(wait_strategy_)
        wait_strategy_->notifyOne();
    return true;
}

template<typename T>
bool BoundedQueue<T>::waitDequeue(T *element){
    while(!break_all_wait_.load(std::memory_order_acquire)){
        /*如果队列里有这个数据，则直接return true，否则返回false*/
        if(Dequeue(element))
            return true;

        if(wait_strategy_ == nullptr)
            return false;

        /*执行等待策略*/
        if(wait_strategy_->emptyWait())
            continue;

        // wait timeout or breakAllWait
        break;
    }

    return false;
} 


template <typename T>
inline uint64_t BoundedQueue<T>::Size() {
    if(!init_)
        return 0;

    uint64_t tail = enqueue_pos_.load(std::memory_order_acquire);
    uint64_t head = dequeue_pos_.load(std::memory_order_acquire);
    if(tail < head)
        return 0;

    uint64_t size = tail - head;
    return size > capacity_ ? capacity_ : size;
}

template <typename T>
inline bool BoundedQueue<T>::Empty() {
    return Size() == 0;
}

/* 由于是无符号整数，所以返回的是索引，类似于取余*/
template <typename T>
inline uint64_t BoundedQueue<T>::getIndex(uint64_t num) const {
    return num % capacity_;
}

template <typename T>
inline uint64_t BoundedQueue<T>::EmptySequence(uint64_t pos) const {
    return pos * 2;
}

template <typename T>
inline uint64_t BoundedQueue<T>::FullSequence(uint64_t pos) const {
    return pos * 2 + 1;
}

template<typename T>
template<typename U>
bool BoundedQueue<T>::EnqueueImpl(U &&element){
    if(!init_)
        return false;

    Cell *cell = nullptr;
    uint64_t pos = enqueue_pos_.load(std::memory_order_relaxed);

    while(true){
        cell = &buffer_[getIndex(pos)];
        uint64_t sequence = cell->sequence.load(std::memory_order_acquire);
        uint64_t expected = EmptySequence(pos);

        if(sequence == expected){
            if(enqueue_pos_.compare_exchange_weak(pos, pos + 1,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed)){
                break;
            }
        }else if(sequence < expected){
            return false;
        }else{
            pos = enqueue_pos_.load(std::memory_order_relaxed);
        }
    }

    cell->data = std::forward<U>(element);
    cell->sequence.store(FullSequence(pos), std::memory_order_release);
    if(wait_strategy_)
        wait_strategy_->notifyOne();
    return true;
}

template <typename T>
inline void BoundedQueue<T>::setWaitStrategy(WaitStrategy* strategy) {
    wait_strategy_.reset(strategy);
}

template <typename T>
inline void BoundedQueue<T>::breakAllWait() {
    break_all_wait_.store(true, std::memory_order_release);
    if(wait_strategy_)
        wait_strategy_->breakAllWait();
}

template <typename T>
inline uint64_t BoundedQueue<T>::Head() {
    return dequeue_pos_.load(std::memory_order_acquire);
}

template <typename T>
inline uint64_t BoundedQueue<T>::Tail() {
    return enqueue_pos_.load(std::memory_order_acquire);
}

template <typename T>
inline uint64_t BoundedQueue<T>::Commit() {
    return enqueue_pos_.load(std::memory_order_acquire);
}

} // namespace base

#endif
