#ifndef COMMON_CONFPARSE_H_
#define COMMON_CONFPARSE_H_

#include <string>
#include "config.hpp"


namespace config {

bool get_main_config_from_file(const std::string& file_path, MainConfig* config);


}


#endif