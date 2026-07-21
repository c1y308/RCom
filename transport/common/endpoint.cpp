#include "endpoint.hpp"
#include "global_data.hpp"

namespace transport {

Endpoint::Endpoint(const RoleAttributes& attr) : enabled_(false), id_() , attr_(attr){

    if(attr_.host_name.empty()){
        attr_.host_name = common::GlobalData::get_instance()->host_name();
    }

    if(!attr.process_id){
        attr_.process_id = common::GlobalData::get_instance()->process_id();
    }

    if(!attr_.id){
        attr_.id = id_.hash_value();
    }

}

Endpoint::~Endpoint() {}


}  // namespace transport
