#ifndef BASE_MACROS_H
#define BASE_MACROS_H


/* 创建一个名为 name 的模板结构体，用于判断 T 中是否含有 func 函数 */
#define DEFINE_TYPE_TRAIT(name, func)                           \
                                                                \
    template<typename T>                                        \
    struct name                                                 \
    {                                                           \
        template<typename CLASS>                                \
        static constexpr bool test(decltype(&CLASS::func)* ){   \
            return true;                                        \
        }                                                       \
                                                                \
        template<typename>                                      \
        static constexpr bool test(...){                        \
            return false;                                       \
        }                                                       \
                                                                \
        static constexpr bool value = test<T>(nullptr);         \
    };                                                          \
                                                                \
    /* name是一个模板类，定义成员时也要带上模板头 */                \
    template<typename T>                                        \
    constexpr bool name<T>::value;                              \


#endif
