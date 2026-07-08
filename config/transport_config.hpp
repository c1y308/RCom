#ifndef CONFIG_TRANSPORT_CONFIG_H_
#define CONFIG_TRANSPORT_CONFIG_H_

#include <string>

namespace config {

//传输层使用的通讯方式
enum OptionalMode{
    HYBRID = 0,
    INTRA = 1,
    SHM = 2,
    RTPS = 3,
};

struct ShmMulticastLocator{
    std::string ip;
    uint32_t    port;
};

struct ShmConfig {
    std::string notifier_type;
    std::string shm_type;
    ShmMulticastLocator shm_locator;
};


struct CommunicationMode{
    OptionalMode same_proc = INTRA;
    OptionalMode diff_proc = SHM;
    OptionalMode diff_host = RTPS;
};


struct TransportConfig{
    ShmConfig shm_conf;
    CommunicationMode communication_mode;
};

    
}


#endif