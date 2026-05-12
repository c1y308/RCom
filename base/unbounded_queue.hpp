#ifndef BASE_UNBOUNDED_QUEUE_H
#define BASE_UNBOUNDED_QUEUE_H

#include <atomic>
#include <cstddef>

namespace base{

template<typename T>
class UnboundedQueue{
    public:
    UnboundedQueue() {Reset();}
    UnboundedQueue& operator=(const UnboundedQueue &other) = delete;  // 禁止拷贝赋值
    UnboundedQueue(const UnboundedQueue &other) = delete;             // 禁止拷贝构造

    ~UnboundedQueue() {Destroy();}

    void Clear(){
        Destroy();
        Reset();
    }

    std::size_t Size() const {return size_.load();}
    bool Empty() const {return size_.load() == 0;}

    void Enqueue(T element){
        auto node = new Node();
        node->data = std::move(element);
        Node *old_tail = tail_.load();

        while(true){
            if(tail_.compare_exchange_strong(old_tail, node)){
                old_tail->next = node;
                old_tail->release();
                size_.fetch_add(1);
                break;
            }
        }
    }

    bool Dequeue(T *element){
        Node *old_head = head_.load();
        Node *head_next = nullptr;

        do{
            head_next = old_head->next;

            if(head_next == nullptr)
                return false;

        }while(!head_.compare_exchange_strong(old_head, head_next));

        *element = std::move(head_next->data);  // 重点改动 move 语句
        size_.fetch_sub(1);
        old_head->release();
        return true;
    }

    private:
    struct Node{
        T data;
        std::atomic<uint32_t> ref_count;
        Node *next = nullptr;

        Node(){ref_count.store(2);}

        void release(){
            if(ref_count.fetch_sub(1) == 1){
                delete this;
            }
        }
    };

    /* 遍历链表节点并删除所有节点 */
    void Destroy(){
        auto it = head_.load();
        while(it != nullptr){
            Node *temp = it->next;
            delete it;
            it = temp;
        }
    }

    /*初始化:创建一个节点*/
    void Reset(){
        auto node = new Node();
        head_ = node;
        tail_ = node;
        size_.store(0);
    }

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<std::size_t> size_;
};

}

#endif
