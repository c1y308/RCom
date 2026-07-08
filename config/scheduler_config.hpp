#ifndef CMW_CONFIG_SCHEDULERCONF_H_
#define CMW_CONFIG_SCHEDULERCONF_H_

#include "classic_config.hpp"
#include <string>
#include <vector>


namespace config {

struct InnerThread {
    std::string name;
    std::string cpuset;
    std::string policy;
    uint32_t prio = 0; // 默认值
};

struct SchedulerConfig {
    std::string policy;
    uint32_t routine_num = 0; // 默认值
    uint32_t default_proc_num = 0; // 默认值
    std::string process_level_cpuset;
    std::vector<InnerThread> threads;
    ClassicConfig classic_conf;
};

}

#endif