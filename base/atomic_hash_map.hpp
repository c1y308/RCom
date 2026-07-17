#ifndef ATOMIC_HASH_MAP_H
#define ATOMIC_HASH_MAP_H

/**
 * @file atomic_hash_map.hpp
 * @author c1y308
 * @brief 提供基于原子链表桶的固定容量哈希表模板。
 */

#include <cstdint>
#include <atomic>
namespace base {

template <typename Key, typename Value, std::size_t TableSize = 128>
class AtomicHashMap {
public:
    AtomicHashMap() : capacity_(TableSize), mode_num_(capacity_ - 1) {}
    /* 禁止拷贝构造和拷贝赋值操作 */
    AtomicHashMap& operator=(const AtomicHashMap& other) = delete;
    AtomicHashMap(const AtomicHashMap& other) = delete;

    ~AtomicHashMap() = default;

    bool has(Key key) const{
        uint64_t index = key & mode_num_;
        return buckets_[index].has(key);
    }

    bool get(Key key, Value **value_ptr) const{
        uint64_t index = key & mode_num_;
        return buckets_[index].get(key, value_ptr);
    }

    bool get(Key key, Value *value) const{
        uint64_t index = key & mode_num_;
        Value *value_ptr = nullptr;
        bool res = buckets_[index].get(key, &value_ptr);
        if(res)
            *value = *value_ptr;
        return res;
    }

    void set(Key key){
        uint64_t index = key & mode_num_;
        buckets_[index].Insert(key);
    }

    void set(Key key, const Value &value){
        uint64_t index = key & mode_num_;
        buckets_[index].Insert(key, value);
    }

    void set(Key key, Value &&value){
        uint64_t index = key & mode_num_;
        buckets_[index].Insert(key, std::forward<Value>(value));
    }

private:
    /* 本质链表节点 */
    struct Entry{
        /* 什么都不传入的默认构造函数 */
        Entry() {};

        /* 只传入key，value默认构造 */
        explicit Entry(Key key) : key(key){
            value_ptr.store(new Value(), std::memory_order_release);
        };
        /* 传入key和value */
        Entry(Key key, Value value) : key(key) {
            value_ptr.store(new Value(value), std::memory_order_release);
        };
        /* value 为万能引用 */
        Entry(Key key, Value&& value) : key(key) {
            value_ptr.store(new Value(std::forward<Value>(value)), std::memory_order_release);
        };

        /* 析构函数 */
        ~Entry() {
            delete value_ptr.load(std::memory_order_acquire);
        };

        Key key = 0;
        std::atomic<Value*> value_ptr = {nullptr};
        std::atomic<Entry*> next = {nullptr};
    };


    class Bucket{
    public:
        /* 默认构造函数就是构造一个 entry 节点 */
        Bucket() : dummy_(new Entry()) {};
        /* 析构函数就是从head开始遍历，挨个删除节点 */
        ~Bucket(){
            Entry *it = dummy_;
            while(it != nullptr){
                Entry *next = it->next.load(std::memory_order_acquire);
                delete it;
                it = next;
            }
        }
        /* dummy_本身不记录任何数据，所以has函数从dummy_的next开始遍历 */
        bool has(Key key) const{
            Entry *it = dummy_->next.load(std::memory_order_acquire);
            while(it != nullptr){
                if(it->key < key){
                    it = it->next.load(std::memory_order_acquire);
                    continue;
                }else{
                    return it->key == key;
                }
            }
            return false;
        }

        /* 查找key对应的前驱节点与节点 */
        bool Find(Key key, Entry **prev_ptr, Entry **target_ptr) const{
            Entry *prev = dummy_;
            Entry *it = dummy_->next.load(std::memory_order_acquire);
            while(it != nullptr){
                if(it->key == key){
                    *prev_ptr = prev;
                    *target_ptr = it;
                    return true;
                }
                else if(it->key < key){
                    prev = it;
                    it = it->next.load(std::memory_order_acquire);
                    continue;
                }else{
                    *prev_ptr = prev;
                    *target_ptr = prev;
                    return false;
                }
            }
            return false;
        }

        void Insert(Key key, const Value &value){
            Entry *prev = nullptr;
            Entry *target = nullptr;
            Entry *new_entry = nullptr;
            Value *new_value = nullptr;
            while(true){
                /* key 存在，更新 value */
                if(Find(key, &prev, &target)){
                    if(!new_value){
                        new_value = new Value(value);
                    }
                    /* 只有一个线程可以更新这个 key - value */
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if(target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_relaxed)){
                        delete old_val_ptr;
                        if(new_entry){
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                }else{
                    if(!new_entry){
                        new_entry = new Entry(key, value);
                    }
                    /* 只有一个线程可以插入这个 key - value */
                    new_entry->next.store(target, std::memory_order_release);
                    if(prev->next.compare_exchange_strong(target, new_entry,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed)){
                        if(new_value){
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                    continue;
                }
            }

        }

        void Insert(Key key, Value &&value){
            Entry *prev = nullptr;
            Entry *target = nullptr;
            Entry *new_entry = nullptr;
            Value *new_value = nullptr;
            while(true){
                /* key 存在，更新 value */
                if(Find(key, &prev, &target)){
                    if(!new_value){
                        new_value = new Value(std::forward<Value>(value));
                    }
                    /* 只有一个线程可以更新这个 key - value */
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if(target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_relaxed)){
                        delete old_val_ptr;
                        if(new_entry){
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                }else{
                    if(!new_entry){
                        new_entry = new Entry(key, std::forward<Value>(value));
                    }
                    /* 只有一个线程可以插入这个 key - value */
                    new_entry->next.store(target, std::memory_order_release);
                    if(prev->next.compare_exchange_strong(target, new_entry,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed)){
                        if(new_value){
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                }
            }

        }

        void Insert(Key key){
            Entry *prev = nullptr;
            Entry *target = nullptr;
            Entry *new_entry = nullptr;
            Value *new_value = nullptr;
            while(true){
                /* key 存在，直接返回 */
                if(Find(key, &prev, &target)){
                    if(!new_value){
                        new_value = new Value();
                    }
                    /* 只有一个线程可以更新这个 key - value */
                    auto old_val_ptr = target->value_ptr.load(std::memory_order_acquire);
                    if(target->value_ptr.compare_exchange_strong(old_val_ptr, new_value, std::memory_order_relaxed)){
                        delete old_val_ptr;
                        if(new_entry){
                            delete new_entry;
                            new_entry = nullptr;
                        }
                        return;
                    }
                    continue;
                }else{
                    if(!new_entry){
                        new_entry = new Entry(key);
                    }
                    /* 只有一个线程可以插入这个 key - value */
                    new_entry->next.store(target, std::memory_order_release);
                    if(prev->next.compare_exchange_strong(target, new_entry,
                                                          std::memory_order_acq_rel,
                                                          std::memory_order_relaxed)){
                        if(new_value){
                            delete new_value;
                            new_value = nullptr;
                        }
                        return;
                    }
                }
            }   
        }

        bool get(Key key, Value **value_ptr) const{
            Entry *prev = nullptr;
            Entry *target = nullptr;
            if(Find(key, &prev, &target)){
                *value_ptr = target->value_ptr.load(std::memory_order_acquire);
                return true;
            }
            return false;
        }
    private:
        /* dummy_本身不记录任何数据 */ 
        Entry *dummy_ = nullptr;
    };
private:
    Bucket buckets_[TableSize];
    uint64_t capacity_;
    uint64_t mode_num_;
};
}
#endif
