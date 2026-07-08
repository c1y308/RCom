#ifndef _COMMON_ENVIRONMENT_H_
#define _COMMON_ENVIRONMENT_H_

#include <string>
#include <iostream>
#include "log.hpp"
namespace common {

/**
 * @brief  获取环境变量
 */
inline std::string get_env(const std::string& var_name,
                           const std::string& default_value = "") {
  const char* var = std::getenv(var_name.c_str());
  if (var == nullptr) {
    AWARN << "Environment variable [" << var_name << "] not set, fallback to " << default_value;  
    return default_value;
  }
  return std::string(var);
}

/**
 * @brief  获取工作空间路径
 */
inline const std::string get_work_root() {
  std::string work_root = get_env("C1Y308_PATH");
  if (work_root.empty()) {
    work_root = "/RCom";
  }
  return work_root;
}

}

#endif