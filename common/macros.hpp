#ifndef COMMON_MACROS_HPP
#define COMMON_MACROS_HPP
#include "../base/macros.hpp"
#include <type_traits>
#include <mutex>
DEFINE_TYPE_TRAIT(has_shutdown, shutdown);

/* 如果 T 有 shutdown 方法，则调用它 */
template<typename T>
std::enable_if_t<has_shutdown<T>::value, void> call_shutdown(T& obj) {
    obj.shutdown();
}

/* 如果 T 没有 shutdown 方法，则什么都不做 */
template<typename T>
std::enable_if_t<!has_shutdown<T>::value, void> call_shutdown(T& obj) {
    // 什么都不做
    (void)obj; // 避免未使用参数的警告
}

#undef UNUSED
#define UNUSED(param) (void)(param)


/*禁用这个类的 拷贝构造 和 拷贝赋值*/
#undef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(class_name)                                            \
    class_name(const class_name&) = delete;                                             \
    class_name& operator=(const class_name&) = delete;


/* 利用std::once_flag 和 std::call_once 实现线程安全的单例类创建 */
#undef  DECLARE_SINGLETON
#define DECLARE_SINGLETON(class_name)                                                   \
public:                                                                                 \
    static class_name* get_instance(bool create_if_needed = true) {                     \
        static class_name *instance = nullptr;                                          \
        if(!instance && create_if_needed == true){                                      \
            static std::once_flag flag;                                                 \
            std::call_once(flag, [&] () {instance = new (std::nothrow) class_name();}); \
        }                                                                               \
        return instance;                                                                \
    }                                                                                   \
                                                                                        \
    static void clean_up(){                                                             \
        auto instance = get_instance(false);                                            \
        if(instance != nullptr){                                                        \
            call_shutdown(instance);                                                    \
        }                                                                               \
    }                                                                                   \
private:                                                                                \
    class_name();                                                                       \
    DISALLOW_COPY_AND_ASSIGN(class_name)

#endif  // COMMON_MACROS_HPP
