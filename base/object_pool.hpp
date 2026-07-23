#ifndef OBJECT_POOL_HPP
#define OBJECT_POOL_HPP

/**
 * @file object_pool.hpp
 * @author c1y308
 * @brief 提供固定数量对象的内存池模板。
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include "for_each.hpp"
namespace base{
template<typename T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
public:
    using init_func_type = std::function<void(T *)>;

    explicit ObjectPool(uint32_t num_objects);

    template<typename... Args>
    explicit ObjectPool(uint32_t num_objects, init_func_type init_func, Args&&... args);

    ~ObjectPool();

    /* 获取一个对象。非空池需要由 std::shared_ptr<ObjectPool<T>> 管理后再调用。 */
    std::shared_ptr<T> GetObject();
private:

    struct Node{
        T object;
        Node *next;

        Node() : object(), next(nullptr) {}
        template<typename... NArgs>
        explicit Node(NArgs&&... nargs) : object(std::forward<NArgs>(nargs)...), next(nullptr) {}
        ~Node() = default;
    };

    static_assert(alignof(Node) <= alignof(std::max_align_t),
                  "Node alignment exceeds malloc guarantee");
    static_assert(std::is_standard_layout<T>::value,
                  "T must be standard layout for ObjectPool");

    ObjectPool(const ObjectPool& other) = delete;
    ObjectPool& operator=(const ObjectPool& other) = delete;

    void ReleaseObject(T *object);

    template<std::size_t... Is, typename Tuple>
    static Node* node_from_tuple(void* slot, Tuple& t, std::index_sequence<Is...>) {
        return new (slot) Node(std::get<Is>(t)...);
    }

    uint32_t objects_num_ = 0;
    char *object_area_ = nullptr;
    Node *free_list_head_ = nullptr;
    std::mutex mutex_;
};

/* 构造函数（无额外参数，T 默认构造） */
template<typename T>
ObjectPool<T>::ObjectPool(uint32_t num_objects) : objects_num_(num_objects) {
    const size_t object_size = sizeof(Node);
    if (objects_num_ == 0) {
        return;
    }
    if (objects_num_ > SIZE_MAX / object_size) {
        throw std::bad_alloc();
    }
    object_area_ = static_cast<char *>(std::malloc(objects_num_ * object_size));
    if(!object_area_){
        throw std::bad_alloc();
    }

    size_t i = 0;
    try {
        for (; i < objects_num_; ++i) {
            Node *node = new (object_area_ + i * object_size) Node;
            node->next = free_list_head_;
            free_list_head_ = node;
        }
    } catch (...) {
        for (size_t j = 0; j < i; ++j) {
            reinterpret_cast<Node *>(object_area_ + j * object_size)->~Node();
        }
        std::free(object_area_);
        object_area_ = nullptr;
        throw;
    }
}

/* 构造函数（带 init_func_type，为每个对象额外调用 init_func） */
template<typename T>
template<typename... Args>
ObjectPool<T>::ObjectPool(uint32_t num_objects, init_func_type init_func, Args&&... args)
    : objects_num_(num_objects) {
    const size_t object_size = sizeof(Node);
    if (objects_num_ == 0) {
        return;
    }
    if (objects_num_ > SIZE_MAX / object_size) {
        throw std::bad_alloc();
    }
    auto stored_args = std::make_tuple(std::forward<Args>(args)...);

    object_area_ = static_cast<char *>(std::malloc(objects_num_ * object_size));
    if(!object_area_){
        throw std::bad_alloc();
    }

    size_t constructed = 0;
    try {
        for (size_t i = 0; i < objects_num_; ++i) {
            Node *node = node_from_tuple(object_area_ + i * object_size, stored_args,
                                         std::index_sequence_for<Args...>{});
            ++constructed;
            init_func(&node->object);
            node->next = free_list_head_;
            free_list_head_ = node;
        }
    } catch (...) {
        for (size_t j = 0; j < constructed; ++j) {
            reinterpret_cast<Node *>(object_area_ + j * object_size)->~Node();
        }
        std::free(object_area_);
        object_area_ = nullptr;
        throw;
    }
}

/* 析构函数 */
template<typename T>
ObjectPool<T>::~ObjectPool() {
    if(object_area_ != nullptr){
        const size_t object_size = sizeof(Node);
        FOR_EACH(i, 0, objects_num_){
            reinterpret_cast<Node *>(object_area_ + i * object_size)->~Node();
        }
        std::free(object_area_);
    }
}

/* 获取一个对象 */
template <typename T>
std::shared_ptr<T> ObjectPool<T>::GetObject() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (free_list_head_ == nullptr) {
        return nullptr;
    }

    auto self = this->shared_from_this();
    Node *node = free_list_head_;
    auto object = std::shared_ptr<T>(&node->object,
                                     [self](T *object) { self->ReleaseObject(object); });
    free_list_head_ = node->next;
    return object;
}

/* 释放一个对象 */
template<typename T>
void ObjectPool<T>::ReleaseObject(T *object) {
    if(object == nullptr){
        return;
    }

    auto *node = reinterpret_cast<Node *>(object);
    std::lock_guard<std::mutex> lock(mutex_);
    node->next = free_list_head_;
    free_list_head_ = node;
}


}  // namespace base

#endif
