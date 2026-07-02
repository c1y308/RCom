#ifndef TRANSPORT_COMMON_ENDPOINT_H_
#define TRANSPORT_COMMON_ENDPOINT_H_


#include "identity.hpp"
#include "../config/role_attributes.hpp"
#include <memory>

namespace transport {

using namespace config;
class Endpoint;
using EndpointPtr = std::shared_ptr<Endpoint>;

class Endpoint
{
    public:
        explicit Endpoint(const RoleAttributes& attr);
        virtual ~Endpoint();
    
        const Identity& id() const{ return id_;}
        const RoleAttributes& attributes() const { return attr_;}

    protected:
        bool enabled_;
        Identity id_;
        RoleAttributes attr_;

};

}

#endif