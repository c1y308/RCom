#ifndef BASE_THREAD_POOL_H
#define BASE_THREAD_POOL_H

#include <atomic>
#include <cstddef>
#include <future>
#include <vector>
#include "bounded_queue.hpp"
#include <thread>
#include <functional>
namespace base{

class ThreadPool{
public:
    explicit ThreadPool(std::size_t num_threads, std::size_t max_task_num = 1000);
    ~ThreadPool();

    template<typename Func, typename... Args>
    auto Enqueue(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type>;

private:
    std::vector<std::thread> workers_;
    BoundedQueue<std::function<void()>> task_queue_;
    std::atomic<bool> stop_;
};

/* 构造函数入参为：1. 线程数 2. 最大任务数 */
inline ThreadPool::ThreadPool(std::size_t num_threads, std::size_t max_task_num) : stop_(false) {
    if(!task_queue_.Init(max_task_num, new BlockWaitStrategy())) {
        throw std::runtime_error("Failed to initialize task queue");
    }

    workers_.reserve(num_threads);
    for (std::size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back([this] {
            while (!stop_) {
                std::function<void()> task;
                if (task_queue_.waitDequeue(&task)) {
                    task();
                }
            }
        });
    }
}

template<typename Func, typename... Args>
auto ThreadPool::Enqueue(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type> {
    using return_type = typename std::result_of<Func(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
    );
    std::future<return_type> future = task->get_future();

    if (stop_) {
        return std::future<return_type>(); // Return an empty future if the pool is stopped
    }
    task_queue_.Enqueue([task]() { (*task)(); });
    return future;
}

/* 唤醒线程池里的所有线程，然后等待所有线程结束，释放资源 */
inline ThreadPool::~ThreadPool() {
    if(stop_.exchange(true)) {
        return; // Already stopped
    }

    task_queue_.breakAllWait();

    for (std::thread &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}


}  // namespace base

#endif // BASE_THREAD_POOL_H
