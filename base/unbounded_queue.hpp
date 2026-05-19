#ifndef BASE_UNBOUNDED_QUEUE_H
#define BASE_UNBOUNDED_QUEUE_H

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
    return size_.load();
}

template<typename T>
bool UnboundedQueue<T>::Empty() const{
    return size_.load() == 0;
}

template<typename T>
void UnboundedQueue<T>::Enqueue(T element){
    auto node = new Node();
    node->data = std::move(element);
    Node *old_tail = tail_.load();

    /* 没抢到是不可以入队的，因此无其他操作 */
    do {
    } while(!tail_.compare_exchange_strong(old_tail, node));

    old_tail->next = node;
    old_tail->release();
    size_.fetch_add(1);
}

template<typename T>
bool UnboundedQueue<T>::Dequeue(T *element){
    Node *old_head = head_.load();
    Node *head_next = nullptr;

    do{
        head_next = old_head->next;

        if(head_next == nullptr)
            return false;

        *element = std::move(head_next->data);  // 重点改动 move 语句
    }while(!head_.compare_exchange_strong(old_head, head_next));

    size_.fetch_sub(1);
    old_head->release();
    return true;
}



template<typename T>
UnboundedQueue<T>::Node::Node(){
    ref_count.store(2);
}

template<typename T>
void UnboundedQueue<T>::Node::release(){
    if(ref_count.fetch_sub(1) == 1){
        delete this;
    }
}


}

#endif
