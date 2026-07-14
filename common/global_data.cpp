#include "global_data.hpp"
#include <unistd.h>
#include "file.hpp"
#include <ifaddrs.h>
#include "log.hpp"
#include <netdb.h>
#include "environment.hpp"
#include "../config/config_parse.hpp"

namespace common{


//类中静态成员是类的一部分，但是不是类的实例的一部分，因此需要在此定义
AtomicHashMap<uint64_t, std::string, 256> GlobalData::channel_id_map_;
AtomicHashMap<uint64_t, std::string, 512> GlobalData::node_id_map_;
AtomicHashMap<uint64_t, std::string, 256> GlobalData::task_id_map_;


//返回当前运行进程的路径
std::string program_path(){
    char path[4096];
    auto len = readlink("proc/self/exe", path , sizeof(path) - 1);
    if(len == -1 ){
        return "";
    }

    path[len] = '\0';
    /* 转换为 string 类型 */
    return std::string(path);
}


/* 1.初始化 2.获取并构建进程信息 */
GlobalData::GlobalData() {
    init_host_info();
    init_config();

    // 获取当前运行进程的 PID
    process_id_ = getpid();
    // 获取当前运行进程的路径
    std::string process_path = program_path();

    if(!process_path.empty()){
        process_group_ = get_file_name(process_path) + "_" + std::to_string(process_id_);
    }else{
        process_group_ = "default_" + std::to_string(process_id_);
    }
}

GlobalData::~GlobalData() {}

void GlobalData::set_component_nums(const int component_nums){
    component_nums_ = component_nums;
}

int GlobalData::component_nums() const { return component_nums_; }



void GlobalData::init_host_info(){

    /* 使用 gethostname 获取主机名（unistd.h） */
    char host_name[1024];
    gethostname(host_name , sizeof(host_name));
    host_name_ = host_name;

    // 使用本地回环ip地址
    host_ip_ = "127.0.0.1";

    const char *ip_env = getenv("C1Y308_IP");
    if(ip_env != nullptr)
    {
        //转换为 string 类型并取IP前三个字符
        std::string ip_env_str(ip_env);
        std::string starts = ip_env_str.substr(0 , 3);
        if(starts != "127"){
            host_ip_ = ip_env_str;
            return;
        }
    }

    ifaddrs* ifaddr = nullptr;
    if(getifaddrs(&ifaddr) != 0)
    {
        AERROR << "getifaddrs failed, we will use 127.0.0.1 as host ip.";
    }
    //ifaddrs 是一个链表，遍历每个节点
    for(ifaddrs* ifa = ifaddr; ifa ; ifa = ifa->ifa_next){

        if(ifa->ifa_addr == nullptr){
            continue;
        }
        int family = ifa->ifa_addr->sa_family;
        if(family  != AF_INET){
            continue;
        }

        //获取本机的IP地址
        char addr[NI_MAXHOST] = {0};
        if(getnameinfo(ifa->ifa_addr , sizeof(sockaddr_in) , addr , NI_MAXHOST , NULL , 
                       0 , NI_NUMERICHOST) != 0){
            continue;
        }

        std::string tmp_ip(addr);
        std::string starts = tmp_ip.substr(0,3);
        if(starts != "127"){
            host_ip_ = tmp_ip;
            break;
        }
    }

    //free 掉链表
    freeifaddrs(ifaddr);

}

int GlobalData::process_id() const { return process_id_; }

void GlobalData::set_process_group(const std::string& process_group) {
  process_group_ = process_group;
}

const std::string& GlobalData::process_group() const { return process_group_; }


const std::string& GlobalData::host_ip() const { return host_ip_; }
const std::string& GlobalData::host_name() const { return host_name_; }



// 读取 Rcom/conf/c1y308.pb.conf并解析
bool GlobalData::init_config() {
    /* 设定配置路径 */
    std::string config_path("RCom/conf/c1y308.pb.conf");
    config_path = get_absolute_path(get_work_root(), config_path);

    if (!config::get_main_config_from_file(config_path, &config_)) {
        AERROR << "Read RCom/conf/c1y308.pb.conf from absolute path failed!";
        return false;
  }
  AINFO << "Read RCom/conf/c1y308.pb.conf from absolute path sucess!";
  return true;
}   

const config::MainConfig& GlobalData::config() const { return config_; }


//注册 Channel
uint64_t GlobalData::register_channel(const std::string& channel) {

  //拿到需要注册的 channel 的哈希值
  auto id = std::hash<std::string>{}(channel);
  //如果channel_id_map_能找到此id
  while (channel_id_map_.has(id)) {
    std::string* name = nullptr;
    channel_id_map_.get(id, &name);
    if (channel == *name) {
      //此channel已经被注册，直接break
      break;
    }
    //说明有其他channel和当前的哈希值相等，出现了碰撞，将id++
    ++id;
    AWARN << "Channel name hash collision: " << channel << " <=> " << *name;
  }

  channel_id_map_.set(id, channel);
  return id;
}

std::string GlobalData::get_channel_by_id(uint64_t id)
{
    std::string* channel = nullptr;
    if(channel_id_map_.get(id, &channel))
    {
        return *channel;
    }
    return "";
}


//注册Node
uint64_t GlobalData::register_node(const std::string& node_name){

    //拿到 node_name 的哈希值
    auto id  = std::hash<std::string>{}(node_name);
    //检查 hashmap 中是否含有 id
    while (node_id_map_.has(id))
    {
       //如果 id 存在
       std::string* name = nullptr;
       node_id_map_.get(id, &name);
       if(node_name == *name){
        break;
       }
       //说明有其他 node_name 和当前的哈希值相等，出现了哈希碰撞，将id++
       ++id;
       AWARN << " Node name hash collision: " << node_name << " <=> " << *name;
    }
    //确保node_name是一个唯一的id
    node_id_map_.set(id , node_name);
    return id;
}


uint64_t GlobalData::register_task_name(const std::string& task_name) {
    auto id = std::hash<std::string>{}(task_name);
    while (task_id_map_.has(id))
    {
        std::string* name = nullptr;
        task_id_map_.get(id, &name);
        if(task_name == *name){
            break;
        }
        ++id;
        AWARN << "Task name hash collision: " << task_name << " <=> " << *name;
    }

    task_id_map_.set(id, task_name);
    return id;
}

std::string GlobalData::get_task_name_by_id(uint64_t id){
    std::string* task_name = nullptr;
    if(task_id_map_.get(id, &task_name)){
        return *task_name;
    }
    return "";
}
}