#ifndef CMW_CONFIG_CMWCONF_H_
#define CMW_CONFIG_CMWCONF_H_

#include "scheduler_config.hpp"
#include "transport_config.hpp"
#include <nlohmann/json.hpp>


namespace config {

struct MainConfig
{
    SchedulerConfig scheduler_conf;
    TransportConfig transport_conf;
};


// nlohmann::json 序列化支持
void from_json(const nlohmann::json& j, InnerThread& thread);
void from_json(const nlohmann::json& j, ClassicTask& task);
void from_json(const nlohmann::json& j, SchedGroup& group);
void from_json(const nlohmann::json& j, ClassicConfig& classic_conf);
void from_json(const nlohmann::json& j, SchedulerConfig& scheduler_conf);
void from_json(const nlohmann::json& j, MainConfig& main_config);

}

#endif