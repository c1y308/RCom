#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <string>

namespace common{


/*
  std::hash<std::string>{} ： 创建一个 std::hash<std::string>的临时对象
  { } 是初始化列表 ， std::hash<std::string> 内部重载了()
  获取一个字符串的哈希值
*/
inline std::size_t str_hash(const std::string& key) {
  return std::hash<std::string>{}(key);
}


}


#endif
