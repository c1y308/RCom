#ifndef BASE_THREAD_POOL_HPP
#define BASE_THREAD_POOL_HPP

/**
 * @file thread_pool.hpp
 * @author c1y308
 * @brief 提供基于有界队列的线程池实现。
 */

#include <atomic>
#include <cstddef>  
#include <vector>
#include <thread>
#include <mutex>
#include "bounded_queue.hpp"
#include "wait_strategy.hpp"
#include <functional>
#include <future>
namespace base {


class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto Enqueue(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> workers_;
    BoundedQueue<std::function<void()>> task_queue_;
    std::atomic_bool is_running_;
    std::mutex enqueue_mutex_;
};

inline ThreadPool::ThreadPool(std::size_t threads, std::size_t max_task_num) : is_running_(true){
    if(!task_queue_.Init(max_task_num, new BlockWaitStrategy()))
    {
        throw std::runtime_error("ThreadPool Init failed");
    }

    for(std::size_t i = 0; i < threads; ++i){
        workers_.emplace_back([this](){
            while(is_running_.load(std::memory_order_acquire)){
                std::function<void()> task;
                if(task_queue_.waitDequeue(&task)){
                    task();
                }
            }
        });
    }
}

inline ThreadPool::~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(enqueue_mutex_);
        if(!is_running_.exchange(false))
            return;
    }

    task_queue_.breakAllWait();

    for(auto& worker : workers_){
        if(worker.joinable()){
            worker.join();
        }
    }

    // 排空队列中剩余的任务，避免静默丢弃
    std::function<void()> task;
    while(task_queue_.Dequeue(&task)){
        task();
    }
}


template<typename F, typename... Args>
auto ThreadPool::Enqueue(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
{
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(func), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    
    {
        std::lock_guard<std::mutex> lock(enqueue_mutex_);
        if(!is_running_.load(std::memory_order_relaxed)){
            std::promise<return_type> promise;
            promise.set_exception(std::make_exception_ptr(
                std::runtime_error("ThreadPool is stopped")));
            return promise.get_future();
        }
        task_queue_.Enqueue([task](){ (*task)(); });
    }

    return res;
}

}

#endif
