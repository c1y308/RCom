# ifndef __GLOBAL_DATA_HPP__
# define __GLOBAL_DATA_HPP__
#include <string>
#include "atomic_hash_map.hpp"
#include "func_trait.hpp"
#include "declare_singleton.hpp"
#include "../config/config.hpp"
using base::AtomicHashMap;
namespace common{
class GlobalData{
public:
    ~GlobalData();

    // 获取运行机器的配置信息
    const std::string& host_ip() const;
    const std::string& host_name() const;

    // 获取当前进程的信息
    int process_id() const; 
    const std::string& process_group() const;
    void set_process_group(const std::string& group);

    // 获取调度策略信息
    const std::string& sched_name() const;

    // 获取当前 config
    const config::MainConfig& config() const;

    void set_component_nums(const int component_nums);
    int component_nums() const;

    //注册channel，返回 channel 的 id
    static uint64_t register_channel(const std::string& channel);

    static std::string get_channel_by_id(uint64_t id);

    //注册node，返回 node 的 id
    static uint64_t register_node(const std::string& node_name);

    static std::string get_task_name_by_id(uint64_t id);

    uint64_t register_task_name(const std::string& task_name);
private:

    void init_host_info();

    //  从配置文件中读取配置信息
    bool init_config();

private:
    // 运行机器的配置信息
    std::string host_ip_;
    std::string host_name_;

    //进程信息
    int process_id_;  // PID
    std::string process_group_;  // 进程组名，格式为：程序名_PID

    int component_nums_;

    // run mode 暂时没有运行模式
     
    // sched policy info  暂时不支持调度
    std::string sched_name_ = "DEFAULT";  

    config::MainConfig config_;

    //在创建新的channel时会注册进此全局map
    static AtomicHashMap<uint64_t, std::string, 256> channel_id_map_;   //全局 channel_id_map_ 表
    static AtomicHashMap<uint64_t, std::string, 512> node_id_map_;
    
    static AtomicHashMap<uint64_t, std::string, 256> task_id_map_;

    //GlobalData为全局单例
    DECLARE_SINGLETON(GlobalData)

};

}
#endif
