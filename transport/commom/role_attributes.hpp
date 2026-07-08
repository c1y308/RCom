#ifndef CMW_CONFIG_ROLEATTRIBUTES_H_
#define CMW_CONFIG_ROLEATTRIBUTES_H_

#include <string>
#include "../../serialize/serializable.hpp"
#include "../../serialize/data_stream.hpp"
#include "../../config/qos_profile.hpp"

namespace transport {
using namespace serialize;

/*继承自Serializable，用于描述节点的属性*/
struct RoleAttributes : public Serializable
{
    std::string host_name;       //主机名
    std::string host_ip;         //主机IP
    int32_t process_id;          //进程ID

    std::string channel_name;    // channel name
    uint64_t channel_id;         // hash value of channel_name

    config::QosProfile qos_profile;      //Qos配置策略
    uint64_t id;                 //

    std::string node_name;       // node name
    uint64_t node_id;            // hash value of node_name
    
    std::string message_type;    // 消息类型

    SERIALIZE(host_name, host_ip, process_id, channel_name,qos_profile, id, node_name, node_id, message_type)
};


}



#endif