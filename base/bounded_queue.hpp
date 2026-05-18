#ifndef BASE_BOUNDED_QUEUE_H
#define BASE_BOUNDED_QUEUE_H
#include <atomic>
#include "wait_strategy.hpp"
#define cyber_unlikely(x) (__builtin_expect((x), 0))
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
    uint64_t getIndex(uint64_t num);

    alignas(CACHELINE_SIZE) std::atomic<uint64_t> head_   = {0};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> tail_   = {1};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> commit_ = {1};

    T *pool_ = nullptr;
    uint64_t pool_size_ = 0;

    std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;  // 指向等待策略类的实例
    volatile bool break_all_wait_ = false;
};


/* 默认线程阻塞策略为睡眠策略 */
template<typename T>
inline bool BoundedQueue<T>::Init(uint64_t size){
    return Init(size, new SleepWaitStrategy());
}

/* 分配内存/设置队列的等待策略 */
template<typename T>
inline bool BoundedQueue<T>::Init(uint64_t size, WaitStrategy *strategy){
    pool_size_ = size + 2;
    /* 拿到分配的内存 */
    pool_ = reinterpret_cast<T*>(std::calloc(pool_size_, sizeof(T)));
    if(pool_ == nullptr)
        return false;
    /* placement new */
    for(int64_t i = 0; i < pool_size_; i++){
        new (&(pool_[i])) T();
    }

    /* 设置策略 */
    wait_strategy_.reset(strategy);
    return true;
}


template<typename T>
bool BoundedQueue<T>::Enqueue(const T &element){
    uint64_t new_tail = 0;
    uint64_t old_tail = tail_.load(std::memory_order_acquire);
    uint64_t old_commit = 0;

    do{
        new_tail = old_tail + 1;
        /* 判断队列是否已满 */
        if(getIndex(new_tail) == getIndex(head_.load(std::memory_order_acquire)))
            return false;
    } while(!tail_.compare_exchange_weak(old_tail, new_tail,
                                            std::memory_order_release,
                                            std::memory_order_acquire));
    pool_[getIndex(old_tail)] = element;

    do {
        old_commit = old_tail;
    } while (cyber_unlikely(!commit_.compare_exchange_weak(old_commit, new_tail,
                                                            std::memory_order_acq_rel,
                                                            std::memory_order_relaxed)));
    wait_strategy_->notifyOne();
    return true;
}

template <typename T>
bool BoundedQueue<T>::waitEnqueue(const T &element) {
  while (!break_all_wait_) {
    if (Enqueue(element)) {
      return true;
    }
    if (wait_strategy_->emptyWait()) {
      continue;
    }
    // wait timeout
    break;
  }

  return false;
}

template<typename T>
bool BoundedQueue<T>::waitEnqueue(T &&element){
    while(!break_all_wait_){
        if(Enqueue(std::move(element)))
            return true;
        if(wait_strategy_->emptyWait())
            continue;
        // wait timeout
        break;
    }
    return false;
}



template<typename T>
bool BoundedQueue<T>::Dequeue(T *element){
    uint64_t new_head = 0;
    uint64_t old_head = head_.load(std::memory_order_acquire);

    do{
        new_head = old_head + 1;
        /* 判断队列是否为空 */
        if(new_head == commit_.load(std::memory_order_acquire))
            return false;

        *element = pool_[getIndex(new_head)];
    } while(!head_.compare_exchange_weak(old_head, new_head,
                                          std::memory_order_release,
                                          std::memory_order_acquire));
    return true;
}

template<typename T>
bool BoundedQueue<T>::waitDequeue(T *element){
    while (!break_all_wait_) {
        /*如果对了里有数据，则直接return true，否则返回false*/
        if (Dequeue(element)) {
        return true;
        }
        /*执行等待策略*/
        if (wait_strategy_->emptyWait()) {
        continue;
        }
        // wait timeout
        break;
  }

  return false;
} 


template <typename T>
inline uint64_t BoundedQueue<T>::Size() {
  return tail_ - head_ - 1;
}

template <typename T>
inline bool BoundedQueue<T>::Empty() {
  return Size() == 0;
}

/* 由于是无符号整数，所以返回的是索引，类似于取余*/
template <typename T>
inline uint64_t BoundedQueue<T>::getIndex(uint64_t num) {
  return num - (num / pool_size_) * pool_size_;  // faster than %
}

template <typename T>
inline void BoundedQueue<T>::setWaitStrategy(WaitStrategy* strategy) {
  wait_strategy_.reset(strategy);
}

template <typename T>
inline void BoundedQueue<T>::breakAllWait() {
  break_all_wait_ = true;
  wait_strategy_->breakAllWait();
}

} // namespace base

#endif