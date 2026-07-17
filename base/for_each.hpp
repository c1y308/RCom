#ifndef FOR_EACH_HPP
#define FOR_EACH_HPP

/**
 * @file for_each.hpp
 * @author c1y308
 * @brief 提供基于比较能力检测的 FOR_EACH 遍历宏。
 */

#include "macros.hpp"
#include <type_traits>
DEFINE_TYPE_TRAIT(HasLess, operator<)

/* 如果两个数据都有 < 操作符重载，则直接通过 < 操作符比较 */
template<typename Value, typename End>
std::enable_if_t<HasLess<Value>::value && HasLess<End>::value, bool>
LessThan(const Value &value, const End &end){
    return value < end;
}

/* 如果两个数据没有 < 操作符重载，则判断是否相等 */
template<typename Value, typename End>
std::enable_if_t<!HasLess<Value>::value || !HasLess<End>::value, bool>
LessThan(const Value &value, const End &end){
    return value != end;
}

/* 如果两个数据都没有 < 或 ！= 操作符重载，则编译时报错 */


/* 如果都重载了 <，就是从 begin开始遍历，直到大于等于end */
/* 编译器会对两者进行隐式转换，找出一个两者都能转换到的公共类型（common type）*/
#define FOR_EACH(i, begin, end) \
    for(auto i = (true ? (begin) : (end)); LessThan(i, end); ++i)

#endif // FOR_EACH_HPP
