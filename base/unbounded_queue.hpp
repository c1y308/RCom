#ifndef BASE_UNBOUNDED_QUEUE_H
#define BASE_UNBOUNDED_QUEUE_H

/**
 * @file unbounded_queue.hpp
 * @author c1y308
 * @brief 提供基于链表节点的无界并发队列模板。
 */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace base{

template<typename T>
class UnboundedQueue{
public:
    UnboundedQueue();
    UnboundedQueue& operator=(const UnboundedQueue &other) = delete;  // 禁止拷贝赋值
    UnboundedQueue(const UnboundedQueue &other) = delete;             // 禁止拷贝构造
    ~UnboundedQueue();

    void Clear();

    std::size_t Size() const;
    bool Empty() const;

    void Enqueue(T element);
    bool Dequeue(T *element);

private:
    struct Node{
        T data;
        std::atomic<uint32_t> ref_count;
        Node *next = nullptr;

        Node();
        void release();
    };

    /* 遍历链表节点并删除所有节点 */
    void Destroy();

    /*初始化:创建一个节点*/
    void Reset();

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<std::size_t> size_;
};

/* 遍历所有节点并删除 */
template<typename T>
void UnboundedQueue<T>::Destroy(){
    auto it = head_.load();
    while(it != nullptr){
        Node *temp = it->next;
        delete it;
        it = temp;
    }
}

/* 创建一个新节点，head和tail都指向它 */
template<typename T>
void UnboundedQueue<T>::Reset(){
    auto node = new Node();
    head_ = node;
    tail_ = node;
    size_.store(0);
}

template<typename T>
UnboundedQueue<T>::UnboundedQueue(){
    Reset();
}

template<typename T>
UnboundedQueue<T>::~UnboundedQueue(){
    Destroy();
}

template<typename T>
void UnboundedQueue<T>::Clear(){
    Destroy();
    Reset();
}

template<typename T>
std::size_t UnboundedQueue<T>::Size() const{
    return size_.load(std::memory_order_relaxed);
}

template<typename T>
bool UnboundedQueue<T>::Empty() const{
    return size_.load(std::memory_order_relaxed) == 0;
}

template<typename T>
void UnboundedQueue<T>::Enqueue(T element){
    auto node = new Node();
    node->data = std::move(element);
    Node *old_tail = tail_.load(std::memory_order_relaxed);

    /* 没有前置判断操作，也没有尝试操作 */
    do {
    } while(!tail_.compare_exchange_strong(old_tail, node));  // 只有一个线程可以抢到这个 tail

    old_tail->next = node;
    old_tail->release();
    size_.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
bool UnboundedQueue<T>::Dequeue(T *element){
    Node *old_head = head_.load(std::memory_order_relaxed);
    Node *head_next = nullptr;

    /* 有前置判断操作，也有尝试操作 */
    while(true){
        head_next = old_head->next;

        if(head_next == nullptr)
            return false;

        *element = std::move(head_next->data);
        if(head_.compare_exchange_strong(old_head, head_next))  // 只有一个线程可以抢到这个 head->next
            break;
    }

    size_.fetch_sub(1, std::memory_order_relaxed);
    old_head->release();
    return true;
}



template<typename T>
UnboundedQueue<T>::Node::Node(){
    ref_count.store(2, std::memory_order_relaxed);
}

template<typename T>
void UnboundedQueue<T>::Node::release(){
    if(ref_count.fetch_sub(1, std::memory_order_relaxed) == 1){
        delete this;
    }
}


}

#endif
